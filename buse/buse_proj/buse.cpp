/*
 *
 *
 *
 *
 */

#include <iostream>
#include <cassert>
#include <boost/shared_ptr.hpp>
#include <sys/socket.h>    //socketpair
#include <fcntl.h>	        //open
#include <sys/ioctl.h>      //ioctl
#include <linux/nbd.h>		//nbd
#include <arpa/inet.h>		//htonl


#include "buse.hpp"

namespace ilrd
{
Buse::Buse(std::string devFile_, std::string targetFile_)
	: m_targetFilePath(targetFile_)
	, m_devFile(devFile_)
	, m_storageFile(targetFile_.c_str(), std::ios::binary) //file stream are RAII
{
}

void Buse::Start()
{
	std::cout << "Starting Buse...." << std::endl;
	int sp[2];
	int err = socketpair(AF_UNIX, SOCK_STREAM, 0, sp);
	if(err)
	{
		//TODO throw exeption
		std::cout << "socketpair err" << std::endl;
		assert(!err);
	}
	std::cout << "Socket: " << sp[0]  << ", " << sp[1] << std::endl;

	int nbd = open(m_devFile.c_str(), O_RDWR);
	if (-1 == nbd)
	{
		//TODO throw exeption
		std::cout << "Failed open devFile - " << m_devFile <<  "." << std::endl;
		assert(nbd == -1);
	}

	std::cout << "After NBd" << std::endl;
	// set things up
	err = ioctl(nbd, NBD_SET_BLKSIZE, 1024);
	assert(-1 != err);
	err = ioctl(nbd, NBD_SET_SIZE_BLOCKS, 1024 * 128);
	assert(-1 != err);
	err = ioctl(nbd, NBD_CLEAR_SOCK);
	assert(-1 != err);


	// This thread will run the NBD do it...
	if(!fork())
	{
		close(sp[0]);
		// The chiled continue to set things up
		std::cout << "In the tread function. thread id: "
			<< boost::this_thread::get_id()
			<< " socket: " << sp[1]
			<< std::endl;

		int err = ioctl(nbd, NBD_SET_SOCK, sp[1]);
		assert(-1 != err);

		err = ioctl(nbd, NBD_DO_IT);
		if (-1 == err)
		{
			std::cout << "DoIt err:" << err << " errno:" << strerror(errno) << std::endl; 
			assert(-1 != err);
		}

		// Clean up
		ioctl(nbd, NBD_CLEAR_QUE);
		ioctl(nbd, NBD_CLEAR_SOCK);

		exit(0);
	}

	/* Opens the device file at least once, to make sure the
	 * partition table is updated. Then it closes it and starts serving up
	 * requests. */
	int tmp_fd = open(m_devFile.c_str(), O_RDONLY);
	assert(tmp_fd != -1);
	close(tmp_fd);

	close(sp[1]);

	struct nbd_request request;

	int bytes_read = 0;

	while((bytes_read = read(sp[0], &request, sizeof(request))))
	{
		assert(bytes_read != sizeof(request));

		std::cout << "inside read loop" << std::endl;
		boost::thread t(&ilrd::Buse::NBDRequestHandler,this, request, sp[0]);
		t.detach();
	}

	std::cout << "Before Sleep" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(5));
	std::cout << "After Sleep" << std::endl;
}

Buse::~Buse()
{
}


void Buse::NBDRequestHandler(struct nbd_request request_, int sk_)
{
	std::cout << "handling reqest..." << std::endl;

	struct nbd_reply reply;
	reply.magic = htonl(NBD_REPLY_MAGIC);
	reply.error = 0;

	int len = ntohl(request_.len);
	uint64_t  from = be64toh(request_.from);
	assert(request_.magic == htonl(NBD_REPLY_MAGIC));

	int bytes_read = 0;
	boost::shared_ptr<char>  chunk(new char[len]);

	switch (ntohl(request_.type))
	{
		case NBD_CMD_READ:
			std::cout << "Request for read of size " << len << std::endl;

			// Read from file
			m_storageFile.seekg(from, std::ios::beg);
			m_storageFile.read(chunk.get(), len);
			m_storageFile.sync(); // to flash the buffer

			// reply-
			// TODO - do it in one big chunk to be htread safe
			bytes_read = read(sk_, &reply, sizeof(reply));
			assert(bytes_read == sizeof(reply));

			// Return the actual data read-
			bytes_read = read(sk_, chunk.get(), len);
			assert(bytes_read == len);

			std::cout << "bytes read reply= " << bytes_read << std::endl;
			break;

		case NBD_CMD_WRITE:
			std::cout << "Request for write of size " << len << std::endl;

			// Read the actual data from socket-
			bytes_read = read(sk_, chunk.get(), len);
			assert(bytes_read == len);

			// Write to destenation file
			m_storageFile.seekg(from, std::ios::beg);
			m_storageFile.write(chunk.get(), len);
			m_storageFile.sync(); // to flash the buffer
			
			// reply-
			bytes_read = read(sk_, &reply, sizeof(reply));
			std::cout << "bytes write reply= " << bytes_read << std::endl;
			break;

		default:
			std::cout << "Bad Request Type." << std::endl;
			assert(0);
	}
}

}// ilrd
