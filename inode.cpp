#include "inode.hpp"

std::ostream& operator<<(std::ostream &ostream, const INode &in)
{
  return ostream << "INode to string";
}
INode::~INode()
{
  std::cout << "~INode" << std::endl;
}
unsigned short INode::flags()
{
  return this->_flags;
}
void INode::flags(unsigned short flags)
{
  this->_flags = flags;
}
unsigned short INode::nlinks()
{
  return this->_nlinks;
}
void INode::nlinks(unsigned short nlinks)
{
  this->_nlinks = nlinks;
}
unsigned int INode::uid()
{
  return this->_uid;
}
void INode::uid(unsigned int uid)
{
  this->_uid = uid;
}
unsigned int INode::gid()
{
  return this->_gid;
}
void INode::gid(unsigned int gid)
{
  this->_gid = gid;
}
unsigned long int INode::size()
{
  return (0L | this->_size1) << 32 | this->_size2;
}
void INode::size(unsigned long int size)
{
  this->_size1 = (unsigned int) ((size & 0xFFFFFFFF00000000L) >> 32);
  this->_size2 = (unsigned int) (size & 0x00000000FFFFFFFFL);
}
std::array<unsigned int, 9> INode::addr()
{
  return this->_addr;
}
void INode::addr(std::array<unsigned int, 9> addr)
{
  std::copy(addr.begin(), addr.end(), this->_addr.begin());
}
unsigned int INode::actime()
{
  return this->_actime;
}
void INode::actime(unsigned int actime)
{
  this->_actime = actime;
}
unsigned int INode::modtime()
{
  return this->_modtime;
}
void INode::modtime(unsigned int modtime)
{
  this->_modtime = modtime;
}