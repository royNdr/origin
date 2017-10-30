#ifndef BUSE_HPP
#define BUSE_HPP

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <fstream>

namespace ilrd
{

class Buse : private boost::noncopyable
{
public:
	explicit Buse(std::string devFile_, std::string targetFile_);
	~Buse();

	void Start();

private:
	void NBDRequestHandler(struct nbd_request request_, int socket_);
	std::string m_targetFilePath;
	std::string m_devFile;
	std::fstream m_storageFile;
};

}// ilrd

#endif // BUSE_HPP
