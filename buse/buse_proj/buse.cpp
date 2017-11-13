/*
 *
 *
 */

#include <iostream>
#include <cassert>
#include <boost/shared_ptr.hpp>
#include <sys/socket.h>     //socketpair
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
	, m_storageFile(targetFile_.c_str(), std::ios::trunc | std::ios::in | std::ios::out) //file stream are RAII
{
    if (!m_storageFile.is_open())
    {
	    std::cout << "Fail to open file " << targetFile_ << std::endl;
        m_storageFile.clear();
        m_storageFile.open(targetFile_.c_str(), std::ios::out);
	    std::cout << "status: " << m_storageFile.is_open() << std::endl;
    }
    else
    {
	    std::cout << "Success!" << std::endl;
    }
}

void Buse::Start()
{
	int sp[2];
	int err = socketpair(AF_UNIX, SOCK_STREAM, 0, sp);
    assert(!err);

	int nbd = open(m_devFile.c_str(), O_RDWR);
	if (-1 == nbd)
    {   //TODO throw exeption
		std::cout << "Failed open devFile - " << m_devFile <<  "." << std::endl;
		assert(nbd == -1);
	}

	// set things up
	//err = ioctl(nbd, NBD_SET_BLKSIZE, 1024);
	//assert(-1 != err);
	//err = ioctl(nbd, NBD_SET_SIZE_BLOCKS, 1024 * 128);
	//assert(-1 != err);

	err = ioctl(nbd, NBD_SET_SIZE, 1024 * 128 * 1024);
	assert(-1 != err);

	err = ioctl(nbd, NBD_CLEAR_SOCK);
	assert(-1 != err);


	// This thread will run the NBD DO_IT...
	if(!fork())
	{
		close(sp[0]);

		// The chiled continue to set things up
		std::cout << "In the fork chiled. thread id: "
			<< boost::this_thread::get_id()
			<< " socket: " << sp[1]
			<< std::endl;

		int err = ioctl(nbd, NBD_SET_SOCK, sp[1]);
		assert(-1 != err);
#if defined NBD_SET_FLAGS && defined NBD_FLAG_SEND_TRIM
        err = ioctl(nbd, NBD_SET_FLAGS, NBD_FLAG_SEND_TRIM);
        assert(-1 != err);
#endif

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
		assert(bytes_read == sizeof(request));

		//boost::thread t(&ilrd::Buse::NBDRequestHandler,this, request, sp[0]);
        NBDRequestHandler(request, sp[0]);
		//t.detach();
	}

	std::cout << "Before Sleep" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(5));
	std::cout << "After Sleep" << std::endl;
}

Buse::~Buse()
{
}

static void PrintReply(const struct nbd_reply& rep_)
{
    std::cout << "*REPLAY* MAGIC: " << ntohl(rep_.magic) << " ERROR: " << ntohl(rep_.error)
        << " HANDLE: " << rep_.handle << std::endl;
}

static void PrintRequest(const struct nbd_request& req_)
{
    std::cout << "*REQEST* MAGIC: " << ntohl(req_.magic) << " TYPE: " << ntohl(req_.type)
        << " HANDLE: " << req_.handle << " FROM: " << be64toh(req_.from)
        << " LEN: " << ntohl(req_.len) << std::endl;
}

void Buse::NBDRequestHandler(struct nbd_request request_, int sk_)
{
	struct nbd_reply reply;
    reply.magic = htonl(NBD_REPLY_MAGIC);
    reply.error = htonl(0);
    memcpy(reply.handle, request_.handle, sizeof(reply.handle));
    
	int len = ntohl(request_.len);
	uint64_t  from = be64toh(request_.from);
	assert(request_.magic == htonl(NBD_REQUEST_MAGIC));

	ssize_t bytes_count = 0;
	boost::shared_ptr<char>  chunk(new char[len + sizeof(reply)]);

    std::cout << std::endl;
    std::cout << "*********************" << std::endl;
    PrintRequest(request_);
    std::cout << "*********************" << std::endl;
    PrintReply(reply);
    std::cout << "*********************" << std::endl;
    std::cout << std::endl;

	switch (ntohl(request_.type))
	{
		case NBD_CMD_READ:

			// put the replay stuct before the data-
            memcpy(chunk.get(), &reply, sizeof(reply));

			// Read from file
			m_storageFile.seekg(from, std::ios::beg);
			m_storageFile.read(chunk.get(), len);
			m_storageFile.sync(); // to flash the buffer

			// Return: reply + actual data read-
			bytes_count = write(sk_, chunk.get(), sizeof(reply) + len);
			assert(bytes_count == (len + static_cast<int>(sizeof(reply))));

			std::cout << "bytes read reply= " << len << std::endl;

			break;

		case NBD_CMD_WRITE:
			std::cout << "Request for write of size " << len << std::endl;

			// Read the actual data from socket-
			bytes_count = read(sk_, chunk.get(), len);
			assert(bytes_count == len);

			// Write to destenation file
			m_storageFile.seekg(from, std::ios::beg);
			m_storageFile.write(chunk.get(), len);
			m_storageFile.sync(); // to flash the buffer
			
			// reply-
			bytes_count = write(sk_, &reply, sizeof(reply));
			std::cout << "bytes write reply= " << bytes_count << std::endl;
			break;

		case NBD_CMD_TRIM:
			// reply-
			bytes_count = write(sk_, &reply, sizeof(reply));
			std::cout << "bytes write reply= " << bytes_count << std::endl;
			break;
            
		default:
			std::cout << "Bad Request Type- " << request_.type << std::endl;
			assert(0);
	}
}

}// ilrd
