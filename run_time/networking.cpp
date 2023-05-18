// Copyright Danyil Melnytskyi 2022-2023
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "AttachA_CXX.hpp"
#include "files.hpp"
bool inited = false;
ProxyClassDefine define_UniversalAddress;
ProxyClassDefine define_TcpNetworkStream;
ProxyClassDefine define_TcpNetworkBlocking;
void init_define_UniversalAddress();
void init_define_TcpNetworkStream();
void init_define_TcpNetworkBlocking();

#if defined(_WIN32) || defined(_WIN64)
#define _WINSOCKAPI_
#define WIN32_LEAN_AND_MEAN
#include "tasks_util/native_workers_singleton.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

#include "networking.hpp"
#include "FuncEnviropment.hpp"
#include "../configuration/agreement/symbols.hpp"
#include <condition_variable>
#include <utf8cpp/utf8.h>
LPFN_ACCEPTEX _AcceptEx;
LPFN_GETACCEPTEXSOCKADDRS _GetAcceptExSockaddrs;
LPFN_CONNECTEX _ConnectEx;
LPFN_TRANSMITFILE _TransmitFile;
LPFN_DISCONNECTEX _DisconnectEx;
WSADATA wsaData;

void init_win_fns(SOCKET sock){
	static bool inited = false;
	if (inited)
		return;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	GUID GuidConnectEx = WSAID_CONNECTEX;
	GUID GuidTransmitFile = WSAID_TRANSMITFILE;
	GUID GuidDisconnectEx = WSAID_DISCONNECTEX;
	DWORD dwBytes = 0;

	if (SOCKET_ERROR == WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &_AcceptEx, sizeof(_AcceptEx), &dwBytes, NULL, NULL))
		throw std::runtime_error("WSAIoctl failed get AcceptEx");
	if (SOCKET_ERROR == WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockaddrs, sizeof(GuidGetAcceptExSockaddrs), &_GetAcceptExSockaddrs, sizeof(_GetAcceptExSockaddrs), &dwBytes, NULL, NULL))
		throw std::runtime_error("WSAIoctl failed get GetAcceptExSockaddrs");
	if (SOCKET_ERROR == WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx), &_ConnectEx, sizeof(_ConnectEx), &dwBytes, NULL, NULL))
		throw std::runtime_error("WSAIoctl failed get ConnectEx");
	if(SOCKET_ERROR == WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidTransmitFile, sizeof(GuidTransmitFile), &_TransmitFile, sizeof(_TransmitFile), &dwBytes, NULL, NULL))
		throw std::runtime_error("WSAIoctl failed get TransmitFile");
	if(SOCKET_ERROR == WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidDisconnectEx, sizeof(GuidDisconnectEx), &_DisconnectEx, sizeof(_DisconnectEx), &dwBytes, NULL, NULL))
		throw std::runtime_error("WSAIoctl failed get DisconnectEx");


	inited = true;
}



using universal_address = sockaddr_storage;
namespace UniversalAddress{
	enum class AddressType : uint32_t{
		IPv4,
		IPv6,
		UNDEFINED = (uint32_t)-1
	};
	ValueItem* _define_to_string(ValueItem* args, uint32_t len){
		if(len < 1)
			throw InvalidArguments("universal_address.to_string, expected 1 argument, got " + std::to_string(len));
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("universal_address.to_string, excepted proxy, got " + enum_to_string(args[0].meta.vtype));
		ProxyClass& proxy = (ProxyClass&)args[0];
		if(proxy.declare_ty != &define_UniversalAddress){
			if(proxy.declare_ty->name != "universal_address")
				throw InvalidArguments("universal_address.to_string, excepted universal_address, got " + proxy.declare_ty->name);
			else
				throw InvalidArguments("universal_address.to_string, excepted universal_address, got non native universal_address");
		}
		auto* address = (universal_address*)proxy.class_ptr;
		std::string result;
		result.resize(INET6_ADDRSTRLEN);
		if(address->ss_family == AF_INET)
			inet_ntop(AF_INET, &((sockaddr_in*)address)->sin_addr, result.data(), INET6_ADDRSTRLEN);
		else if(address->ss_family == AF_INET6){
			inet_ntop(AF_INET6, &((sockaddr_in6*)address)->sin6_addr, result.data(), INET6_ADDRSTRLEN);
			if(result.starts_with("::ffff:"))
				result = result.substr(7);
		}
		else
			throw std::runtime_error("universal_address.to_string, unknown address family");
		result.resize(strlen(result.data()));
		
		return new ValueItem(result);
	}
	ValueItem* _define_type(ValueItem* args,uint32_t len){
		if(len < 1)
			throw InvalidArguments("universal_address.type, expected 1 argument, got " + std::to_string(len));
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("universal_address.type, excepted proxy, got " + enum_to_string(args[0].meta.vtype));
		ProxyClass& proxy = (ProxyClass&)args[0];
		if(proxy.declare_ty != &define_UniversalAddress){
			if(proxy.declare_ty->name != "universal_address")
				throw InvalidArguments("universal_address.type, excepted universal_address, got " + proxy.declare_ty->name);
			else
				throw InvalidArguments("universal_address.type, excepted universal_address, got non native universal_address");
		}
		auto* address = (universal_address*)proxy.class_ptr;
		std::string result;
		result.resize(INET6_ADDRSTRLEN);
		if(address->ss_family == AF_INET)
			return new ValueItem((uint32_t)AddressType::IPv4);
		else if(address->ss_family == AF_INET6){
			inet_ntop(AF_INET6, &((sockaddr_in6*)address)->sin6_addr, result.data(), INET6_ADDRSTRLEN);
			if(result.starts_with("::ffff:"))
				return new ValueItem((uint32_t)AddressType::IPv4);
			else
				return new ValueItem((uint32_t)AddressType::IPv6);
		}
		else
			throw std::runtime_error("universal_address.type, unknown address family");
	}
	ValueItem* _define_actual_type(ValueItem* args,uint32_t len){
		if(len < 1)
			throw InvalidArguments("universal_address.actual_type, expected 1 argument, got " + std::to_string(len));
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("universal_address.actual_type, excepted proxy, got " + enum_to_string(args[0].meta.vtype));
		ProxyClass& proxy = (ProxyClass&)args[0];
		if(proxy.declare_ty != &define_UniversalAddress){
			if(proxy.declare_ty->name != "universal_address")
				throw InvalidArguments("universal_address.actual_type, excepted universal_address, got " + proxy.declare_ty->name);
			else
				throw InvalidArguments("universal_address.actual_type, excepted universal_address, got non native universal_address");
		}
		auto* address = (universal_address*)proxy.class_ptr;
		if(address->ss_family == AF_INET)
			return new ValueItem((uint32_t)AddressType::IPv4);
		else if(address->ss_family == AF_INET6){
			return new ValueItem((uint32_t)AddressType::IPv6);
		}
		else
			throw std::runtime_error("universal_address.actual_type, unknown address family");
	}
	ValueItem* _define_port(ValueItem* args,uint32_t len){
		if(len < 1)
			throw InvalidArguments("universal_address.port, expected 1 argument, got " + std::to_string(len));
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("universal_address.port, excepted proxy, got " + enum_to_string(args[0].meta.vtype));
		ProxyClass& proxy = (ProxyClass&)args[0];
		if(proxy.declare_ty != &define_UniversalAddress){
			if(proxy.declare_ty->name != "universal_address")
				throw InvalidArguments("universal_address.port, excepted universal_address, got " + proxy.declare_ty->name);
			else
				throw InvalidArguments("universal_address.port, excepted universal_address, got non native universal_address");
		}
		auto* address = (universal_address*)proxy.class_ptr;
		if(address->ss_family == AF_INET)
			return new ValueItem((uint32_t)((sockaddr_in*)address)->sin_port);
		else if(address->ss_family == AF_INET6){
			return new ValueItem((uint32_t)((sockaddr_in6*)address)->sin6_port);
		}
		else
			throw std::runtime_error("universal_address.port, unknown address family");
	}
	ValueItem* _define_full_address(ValueItem* args, uint32_t len){
		if(len < 1)
			throw InvalidArguments("universal_address.to_string, expected 1 argument, got " + std::to_string(len));
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("universal_address.to_string, excepted proxy, got " + enum_to_string(args[0].meta.vtype));
		ProxyClass& proxy = (ProxyClass&)args[0];
		if(proxy.declare_ty != &define_UniversalAddress){
			if(proxy.declare_ty->name != "universal_address")
				throw InvalidArguments("universal_address.to_string, excepted universal_address, got " + proxy.declare_ty->name);
			else
				throw InvalidArguments("universal_address.to_string, excepted universal_address, got non native universal_address");
		}
		auto* address = (universal_address*)proxy.class_ptr;
		std::string result;
		result.resize(INET6_ADDRSTRLEN);

		auto actual_family = address->ss_family;
		if(address->ss_family == AF_INET)
			inet_ntop(AF_INET, &((sockaddr_in*)address)->sin_addr, result.data(), INET6_ADDRSTRLEN);
		else if(address->ss_family == AF_INET6){
			inet_ntop(AF_INET6, &((sockaddr_in6*)address)->sin6_addr, result.data(), INET6_ADDRSTRLEN);
			if(result.starts_with("::ffff:")){
				result = result.substr(7);
				actual_family = AF_INET;
			}
		}
		else
			throw std::runtime_error("universal_address.to_string, unknown address family");
		result.resize(strlen(result.data()));
		if(actual_family == AF_INET){
			result += ":" + std::to_string((uint32_t)htons(((sockaddr_in*)address)->sin_port));
		}else{
			result = '[' + result +  "]:" + std::to_string((uint32_t)htons(((sockaddr_in6*)address)->sin6_port));
		}
		return new ValueItem(result);
	}
}
void init_define_UniversalAddress(){
	if(!define_UniversalAddress.name.empty())
		return;
	define_UniversalAddress.name = "universal_address";
	define_UniversalAddress.copy = AttachA::Interface::special::proxyCopy<universal_address, false>;
	define_UniversalAddress.destructor = AttachA::Interface::special::proxyDestruct<universal_address, false>;
	define_UniversalAddress.funs[symbols::structures::convert::to_string] = 
		ClassFnDefine(
			new FuncEnviropment(UniversalAddress::_define_to_string, false), false, ClassAccess::pub
		);
	define_UniversalAddress.funs["type"] = 
		ClassFnDefine(
			new FuncEnviropment(UniversalAddress::_define_type, false), false, ClassAccess::pub
		);
	define_UniversalAddress.funs["actual_type"] = 
		ClassFnDefine(
			new FuncEnviropment(UniversalAddress::_define_actual_type, false), false, ClassAccess::pub
		);
	define_UniversalAddress.funs["port"] = 
		ClassFnDefine(
			new FuncEnviropment(UniversalAddress::_define_port, false), false, ClassAccess::pub
		);
	define_UniversalAddress.funs["full_address"] =
		ClassFnDefine(
			new FuncEnviropment(UniversalAddress::_define_full_address, false), false, ClassAccess::pub
		);
}

void internal_makeIP4(universal_address& addr_storage, const char* ip, uint16_t port){
	sockaddr_in6 addr6;
	memset(&addr6, 0, sizeof(addr6));
	addr6.sin6_family = AF_INET6;
	addr6.sin6_port = htons(port);
	addr6.sin6_addr.s6_addr[10] = 0xFF;
	addr6.sin6_addr.s6_addr[11] = 0xFF;
	if(inet_pton(AF_INET, ip, &addr6.sin6_addr.s6_addr[12]) != 1)
		throw InvalidArguments("Invalid ip4 address");

	memcpy(&addr_storage, &addr6, sizeof(addr6));
}
void internal_makeIP6(universal_address& addr_storage, const char* ip, uint16_t port){
	sockaddr_in6 addr6;
	memset(&addr6, 0, sizeof(addr6));
	addr6.sin6_family = AF_INET6;
	addr6.sin6_port = htons(port);
	if(inet_pton(AF_INET6, ip, &addr6.sin6_addr) != 1)
		throw InvalidArguments("Invalid ip6 address");

	memcpy(&addr_storage, &addr6, sizeof(addr6));
}
void internal_makeIP(universal_address& addr_storage, const char* ip, uint16_t port){
	sockaddr_in6 addr6;
	memset(&addr6, 0, sizeof(addr6));
	addr6.sin6_family = AF_INET6;
	addr6.sin6_port = htons(port);
	addr6.sin6_addr.s6_addr[10] = 0xFF;
	addr6.sin6_addr.s6_addr[11] = 0xFF;
	if(inet_pton(AF_INET, ip, &addr6.sin6_addr+12) == 1);
	else if(inet_pton(AF_INET6, ip, &addr6.sin6_addr) != 1)
		throw InvalidArguments("Invalid ip4 address");
	memcpy(&addr_storage, &addr6, sizeof(addr6));
}

void internal_makeIP4_port(universal_address& addr_storage, const char* ip_port){
	const char* port = strchr(ip_port, ':');
	if(!port)
		throw InvalidArguments("Invalid ip4 address");
	uint16_t port_num = (uint16_t)std::stoi(port+1);
	std::string ip(ip_port, port);
	internal_makeIP4(addr_storage, ip.c_str(), port_num);
}
void internal_makeIP6_port(universal_address& addr_storage, const char* ip_port){
	if(ip_port[0] != '[')
		throw InvalidArguments("Invalid ip6:port address");
	const char* port = strchr(ip_port, ']');
	if(!port)
		throw InvalidArguments("Invalid ip6:port address");
	if(port[1] != ':')
		throw InvalidArguments("Invalid ip6:port address");
	if(port[2] == 0 )
		throw InvalidArguments("Invalid ip6:port address");
	uint16_t port_num = (uint16_t)std::stoi(port+2);


	if(ip_port == port - 1){
		sockaddr_in6 addr6;
		memset(&addr6, 0, sizeof(addr6));
		addr6.sin6_family = AF_INET6;
		addr6.sin6_port = htons(port_num);
		memcpy(&addr_storage, &addr6, sizeof(addr6));
		return;
	}
	std::string ip(ip_port+1, port);
	internal_makeIP6(addr_storage, ip.c_str(), port_num);
}
void internal_makeIP_port(universal_address& addr_storage, const char* ip_port){
	if(ip_port[0] == '[')
		return internal_makeIP6_port(addr_storage, ip_port);
	else
		return internal_makeIP4_port(addr_storage, ip_port);
}


void get_address_from_valueItem(ValueItem& ip_port, sockaddr_storage& addr_storage){
	if(ip_port.meta.vtype == VType::proxy){
		ProxyClass& proxy = (ProxyClass&)ip_port;
		if(proxy.declare_ty != &define_UniversalAddress){
			if(proxy.declare_ty->name != "universal_address")
				throw InvalidArguments("excepted universal_address, got " + proxy.declare_ty->name);
			else
				throw InvalidArguments("excepted universal_address, got non native universal_address");
		}
		memcpy(&addr_storage, proxy.class_ptr, sizeof(addr_storage));
	}else if(ip_port.meta.vtype == VType::string){
		universal_address addr;
		internal_makeIP_port(addr, ((std::string)ip_port).c_str());
		memcpy(&addr_storage, &addr, sizeof(addr_storage));
	}else
		throw InvalidArguments("excepted universal_address or string, got " + enum_to_string(ip_port.meta.vtype));
}

#pragma region TCP
struct tcp_handle : public NativeWorkerHandle {
	std::list<std::tuple<char*, size_t>> write_queue;
	std::list<std::tuple<char*, size_t>> read_queue;
	typed_lgr<Task> notify_task;
	SOCKET socket;
	WSABUF buffer;
	char* data;
    int total_bytes;
    int sent_bytes;
	int readed_bytes;
	int data_len;
	bool force_mode;
	uint32_t max_read_queue_size;
	enum class error : uint8_t{
		none = 0,
		remote_close = 1,
		local_close = 2,
		local_reset = 3,
		read_queue_overflow = 4,
		invalid_state = 5,
		undefined_error = 0xFF
	} invalid_reason = error::none;
	enum class Opcode : uint8_t{
		ACCEPT,
		READ,
		WRITE,
		TRANSMIT_FILE,
		INTERNAL_READ,
		INTERNAL_CLOSE
	} opcode = Opcode::ACCEPT;

	tcp_handle(SOCKET socket, int32_t buffer_len, NativeWorkerManager* manager, uint32_t read_queue_size = 10) : socket(socket), NativeWorkerHandle(manager), max_read_queue_size(read_queue_size){
		if(buffer_len < 0)
			throw InvalidArguments("buffer_len must be positive");
		SecureZeroMemory(&overlapped, sizeof(OVERLAPPED));
		data = new char[buffer_len];
		buffer.buf = data;
		buffer.len = buffer_len;
		data_len = buffer_len;
		total_bytes = 0;
		sent_bytes = 0;
		readed_bytes = 0;
		force_mode = false;
	}
	~tcp_handle(){
		close();
	}
	uint32_t available_bytes(){
		if(!data)
			return 0;
		if(readed_bytes)
			return true;
		DWORD value = 0;
		int result = ::ioctlsocket(socket, FIONREAD, &value);
		if(result == SOCKET_ERROR)
			return 0;
		else
			return value;
	}
	bool data_available(){
		return available_bytes() > 0;
	}
	void send_data(const char* data, int len){
		if(!data)
			return;
		char* new_data = new char[len];
		memcpy(new_data, data, len);
		write_queue.push_back(std::make_tuple(new_data, len));
	}
	//async
	bool send_queue_item(){
		if(!data)
			return false;
		if(write_queue.empty())
			return false;
		auto item = write_queue.front();
		write_queue.pop_front();
		auto& send_data = std::get<0>(item);
		auto& val_len = std::get<1>(item);
		std::unique_ptr<char[]> send_data_ptr(send_data);
		//set buffer
		buffer.len = data_len;
		buffer.buf = data;
		while(val_len) {
			size_t to_sent_bytes = val_len > data_len ? data_len : val_len;
			memcpy(data, send_data, to_sent_bytes);
			buffer.len = to_sent_bytes;
			buffer.buf = data;
			if(!send_await()){
				return false;
			}
			if(val_len < sent_bytes)
				return true;
			val_len -= sent_bytes;
			send_data += sent_bytes;
		}
		return true;
	}


	void read_force(uint32_t buffer_len, char* buffer){
		if(!data)
			return;
		if(!buffer_len)
			return;
		if(!buffer)
			return;
		while(buffer_len){
			int readed = 0;
			read_available(buffer, buffer_len, readed);
			buffer += readed;
			if(readed>buffer_len)
				return;
			buffer_len -= readed;
		}
	}
	int64_t write_force(char* to_write, uint32_t to_write_len){
		if(!data)
			return -1;
		if(!to_write_len)
			return -1;
		if(!to_write)
			return -1;

		force_mode = true;
		if(data_len < to_write_len){
			buffer.len = data_len;
			buffer.buf = this->data;
			if(!send_await())
				return -1;
			force_mode = false;
			return sent_bytes;
		}
		else{
			buffer.len = to_write_len;
			buffer.buf = this->data;
			memcpy(this->data, to_write, to_write_len);
			if(!send_await())
				return -1;
			force_mode = false;
			return sent_bytes;
		}
	}


	void read_data(){
		if(!data)
			return;
		typed_lgr<Task> task_await = notify_task = Task::dummy_task();
		opcode = Opcode::READ;
		read();
		Task::await_task(task_await, false);
		notify_task = nullptr;
	}
	void read_available_no_block(char* extern_buffer, int buffer_len, int& readed){
		if(!readed_bytes)
			readed = 0;
		else if(readed_bytes < buffer_len){
			readed = readed_bytes;
			memcpy(extern_buffer, data, readed_bytes);
			readed_bytes = 0;
		}
		else{
			readed = buffer_len;
			memcpy(extern_buffer, buffer.buf, buffer_len);
			readed_bytes -= buffer_len;
			buffer.buf += buffer_len;
			buffer.len -= buffer_len;
		}
	}
	void read_available(char* extern_buffer, int buffer_len, int& readed){
		if(!readed_bytes){
			if(read_queue.empty())
				read_data();
			else{
				auto item = read_queue.front();
				read_queue.pop_front();
				auto& read_data = std::get<0>(item);
				auto& val_len = std::get<1>(item);
				std::unique_ptr<char[]> read_data_ptr(read_data);
				buffer.buf  = data;
				buffer.len = data_len;
				readed_bytes = val_len;
				memcpy(data, read_data, val_len);
			}
		}
		if(readed_bytes < buffer_len){
			readed = readed_bytes;
			memcpy(extern_buffer, data, readed_bytes);
			readed_bytes = 0;
		}
		else{
			readed = buffer_len;
			memcpy(extern_buffer, buffer.buf, buffer_len);
			readed_bytes -= buffer_len;
			buffer.buf += buffer_len;
			buffer.len -= buffer_len;
		}
	}
	char* read_available_no_copy(int& readed){
		if(!readed_bytes)
			read_data();
		readed = readed_bytes;
		readed_bytes = 0;
		return data;
	}


	void close(error err = error::local_close){
		if(!data)
			return;
		pre_close(err);
		internal_close();
	}

	void handle(unsigned long dwBytesTransferred){
		DWORD flags = 0, bytes = 0;
		if(!data)
			return;
		switch (opcode) {
		case Opcode::READ:
			readed_bytes = dwBytesTransferred;
			Task::start(notify_task);
			break;
		case Opcode::WRITE:
			sent_bytes+=dwBytesTransferred;
			if(sent_bytes < total_bytes){
				buffer.buf = data + sent_bytes;
				buffer.len = total_bytes - sent_bytes;
				if(!data_available()){
					if(!send())
						Task::start(notify_task);
				}
				else{
					char* data = new char[buffer.len];
					memcpy(data, buffer.buf, buffer.len);
					write_queue.push_front(std::make_tuple(data, buffer.len));
					if(force_mode){
						opcode = Opcode::INTERNAL_READ;
						read();
					}
					else
						Task::start(notify_task);
				}
			}
			else Task::start(notify_task);
		case Opcode::TRANSMIT_FILE:
			Task::start(notify_task);
			break;
		case Opcode::INTERNAL_READ:
			if(dwBytesTransferred){
				char* buffer = new char[dwBytesTransferred];
				memcpy(buffer, data, dwBytesTransferred);
				read_queue.push_back(std::make_tuple(buffer, dwBytesTransferred));
			}
			if(!data_available()){
				if(read_queue.size() > max_read_queue_size)
					 close(error::read_queue_overflow);
				else read();
			}
			else{
				if(write_queue.empty())
					close(error::invalid_state);
				else{
					auto item = write_queue.front();
					write_queue.pop_front();
					auto& write_data = std::get<0>(item);
					auto& val_len = std::get<1>(item);
					memcpy(data, write_data, val_len);
					delete[] write_data;
					buffer.buf = data;
					buffer.len = val_len;
					if(!send())
						Task::start(notify_task);
				}
			}
			break;
		case Opcode::INTERNAL_CLOSE:
			closesocket(socket);
			socket = INVALID_SOCKET;
			Task::start(notify_task);
			break;
		default:
			break;
		}
	}

	void send_and_close(char* data, int len){
		if(!data)
			return;
		buffer.len = data_len;
		buffer.buf = this->data;
		write_queue ={};
		force_mode = true;
		while(data_len < len) {
			memcpy(buffer.buf, data, buffer.len);
			if(!send_await())
				return;
			data += buffer.len;
			len -= buffer.len;
		}
		if(len){
			//send last part of data and close
			memcpy(buffer.buf, data, len);
			buffer.len = len;
			send_await();
		}
		force_mode = false;
		close();
	}


	bool send_file(void* file, uint64_t data_len, uint64_t offset, uint32_t chunks_size){
		if(!data)
			return false;
		if(chunks_size == 0)
			chunks_size = 0x1000;
		if(data_len == 0){
			return transfer_file(socket, file, data_len, chunks_size, offset);
		}

		if(data_len > 0x7FFFFFFE){
			//send file in chunks using TransmitFile
			uint64_t sended = 0;
			uint64_t blocks = data_len / 0x7FFFFFFE;
			uint64_t last_block = data_len % blocks;

			while(blocks--) 
				if(!transfer_file(socket, file, blocks, chunks_size, sended + offset))
					return false;
					
			
			if(last_block)
				if(!transfer_file(socket, file, data_len, chunks_size, sended + offset))
					return false;
		}else{
			if(!transfer_file(socket, file, data_len, chunks_size, offset))
				return false;
		}
		return true;
	}
	bool send_file(const char* path, size_t path_len, uint64_t data_len, uint64_t offset, uint32_t chunks_size){
		if(!data)
			return false;
		if(chunks_size == 0)
			chunks_size = 0x1000;
		std::u16string wpath;
		utf8::utf8to16(path, path + path_len, std::back_inserter(wpath));
		HANDLE file = CreateFileW((wchar_t*)wpath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if(!file)
			return false;
		bool result;
		try{
			result = send_file(file, data_len, offset, chunks_size);
		}catch(...){
			CloseHandle(file);
			throw;
		}
		CloseHandle(file);
		return result;
	}

	bool valid(){
		return data != nullptr;
	}

	void reset(){
		if(!data)
			return;
		pre_close(error::local_reset);
		closesocket(socket);//with iocp socket not send everything and cancel all operations
	}
	void connection_reset(){
		data = nullptr;
		invalid_reason = error::remote_close;
		if(notify_task)
			Task::start(notify_task);
		notify_task = nullptr;
		readed_bytes = 0;
		internal_close();
	}
	void rebuffer(int32_t buffer_len){
		if(!data)
			return;
		if(buffer_len < 0)
			throw InvalidArguments("buffer_len must be positive");
		char* new_data = new char[buffer_len];
		delete[] data;
		data = new_data;
		data_len = buffer_len;
	}

private:
	void pre_close(error err){
		std::list<std::tuple<char*, size_t>> clear_write_queue;
		write_queue.swap(clear_write_queue);
		for(auto& item : clear_write_queue)
			delete[] std::get<0>(item);
		readed_bytes = 0;
		sent_bytes = 0;
		delete[] data;
		data = nullptr;
		invalid_reason = err;
		if(notify_task)
			Task::start(notify_task);
		notify_task = nullptr;
	}
	void internal_close(){
		typed_lgr<Task> task_await = notify_task = Task::dummy_task();
		opcode = Opcode::INTERNAL_CLOSE;
		if(!_DisconnectEx(socket, nullptr, TF_REUSE_SOCKET, 0)){
			if(WSAGetLastError() != ERROR_IO_PENDING)
				invalid_reason = error::local_close;
		}
		Task::await_task(task_await);
		notify_task = nullptr;
	}
	bool handle_error(){
		auto error = WSAGetLastError();
		if(WSA_IO_PENDING == error) return true;
		else {
			switch (error){
			case WSAECONNRESET:
				invalid_reason = error::remote_close;
				break;
			case WSAECONNABORTED:
			case WSA_OPERATION_ABORTED:
			case WSAENETRESET:
				invalid_reason = error::local_close;
				break;
			case WSAEWOULDBLOCK: return false;//try later
			default:
				invalid_reason = error::undefined_error;
				break;
			}
			close();
			return false;
		}
	}
	void read(){
		DWORD flags = 0;
		buffer.buf = this->data;
		buffer.len = data_len;
		int result = WSARecv(socket, &buffer, 1, NULL, &flags, &overlapped, NULL);
		if ((SOCKET_ERROR == result)){
			handle_error();
			return;
		}
	}
	bool send(){
		DWORD flags = 0;
		opcode = Opcode::WRITE;
		int result = WSASend(socket, &buffer, 1, NULL, flags, &overlapped, NULL);
		if ((SOCKET_ERROR == result))return handle_error();
		return true;
	}
	bool send_await(){
		typed_lgr<Task> task_await = notify_task = Task::dummy_task();
		if(!send())
			return false;
		Task::await_task(task_await);
		notify_task = nullptr;
		return data;//if data is null, then socket is closed
	}
	bool transfer_file(SOCKET sock, HANDLE FILE, uint32_t block, uint32_t chunks_size, uint64_t offset){
		overlapped.Offset = offset & 0xFFFFFFFF;
		overlapped.OffsetHigh = offset >> 32;
		typed_lgr<Task> task_await = notify_task = Task::dummy_task();
		opcode = Opcode::TRANSMIT_FILE;
		bool res = _TransmitFile(sock, FILE, block, chunks_size, &overlapped, NULL, TF_USE_KERNEL_APC);
		if(!res && WSAGetLastError() != WSA_IO_PENDING)
			res = false;
		Task::await_task(task_await);
		notify_task = nullptr;
		return res;
	}
};




#pragma region TcpNetworkStream
class TcpNetworkStream{
	friend class TcpNetworkManager;
	struct tcp_handle* handle;
	TcpNetworkStream(tcp_handle* handle):handle(handle){}
	TaskMutex mutex;
	tcp_handle::error last_error;
	bool checkup(){
		if(!handle)
			return false;
		if(!handle->valid()){
			last_error = handle->invalid_reason;
			delete handle;
			handle = nullptr;
			return false;
		}
		return true;
	}
public:
	~TcpNetworkStream(){
		if(handle){
			std::lock_guard lg(mutex);
			handle->close();
		}
		handle = nullptr;
	}

	ValueItem read_available_ref(){
		std::lock_guard lg(mutex);
		if(!handle)
			return nullptr;
		while(!handle->data_available()){
			if(!handle->send_queue_item())
				break;
		}
		if(!checkup())
			return ValueItem(nullptr, ValueMeta(VType::raw_arr_ui8, false, false, 0) , as_refrence);
		int readed = 0; 
		char* data = handle->read_available_no_copy(readed);
		return ValueItem(data, ValueMeta(VType::raw_arr_ui8, false, false, readed) , as_refrence);
	}
	ValueItem read_available(char* buffer, int buffer_len){
		std::lock_guard lg(mutex);
		if(!handle)
			return nullptr;
		while(!handle->data_available()){
			if(!handle->send_queue_item())
				break;
		}
		
		if(!checkup())
			return (uint32_t)0;
		int readed = 0; 
		handle->read_available(buffer, buffer_len, readed);
		return ValueItem((uint32_t)readed);
	}
	bool data_available(){
		std::lock_guard lg(mutex);
		if(handle)
			return handle->data_available();
		return false;
	}
	void write(char* data, size_t size){
		std::lock_guard lg(mutex);
		if(handle){
			handle->send_data(data, size);
			while(!handle->data_available()){
				if(!handle->send_queue_item())
					break;
			}
			checkup();
		}
	}
	bool write_file(char* path, size_t path_len, uint64_t data_len, uint64_t offset, uint32_t chunks_size){
		std::lock_guard lg(mutex);
		if(handle){
			while(handle->valid())if(!handle->send_queue_item())break;
			
			if(!checkup())
				return false;
			
			return handle->send_file(path, path_len, data_len, offset, chunks_size);
		}
		return false;
	}
	bool write_file(void* fhandle, uint64_t data_len, uint64_t offset, uint32_t chunks_size){
		std::lock_guard lg(mutex);
		if(handle){
			while(handle->valid())if(!handle->send_queue_item())break;
			if(!checkup())
				return false;
			return handle->send_file(fhandle, data_len, offset, chunks_size);
		}
		return false;
	}
	//write all data from write_queue
	void force_write(){
		std::lock_guard lg(mutex);
		if(handle){
			while(handle->valid())if(!handle->send_queue_item())break;
			checkup();
		}
	}
	void force_write_and_close(char* data, size_t size){
		std::lock_guard lg(mutex);
		if(handle){
			handle->send_and_close(data, size);
			last_error = handle->invalid_reason;
			delete handle;
		}
		handle = nullptr;
	}
	void close(){
		std::lock_guard lg(mutex);
		if(handle){
			handle->close();
			last_error = handle->invalid_reason;
			delete handle;
		}
		handle = nullptr;
	}	
	void reset(){
		std::lock_guard lg(mutex);
		if(handle){
			handle->reset();
			last_error = handle->invalid_reason;
			delete handle;
		}
		handle = nullptr;
	}
	void rebuffer(int32_t new_size){
		std::lock_guard lg(mutex);
		if(handle)
			handle->rebuffer(new_size);
	}
	bool is_closed(){
		std::lock_guard lg(mutex);
		if(handle){
			bool res = handle->valid();
			if(!res){
				delete handle;
				handle = nullptr;
			}
			return !res;
		}
		return true;
	}
	tcp_handle::error error(){
		std::lock_guard lg(mutex);
		if(handle)
			return handle->invalid_reason;
		return last_error;
	}
};

ValueItem* funs_TcpNetworkStream_read_available_ref(ValueItem* args, uint32_t len){
	if(len >= 1){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		return new ValueItem(((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->read_available_ref());
	}else
		throw InvalidArguments("The number of arguments must be 1.");
}
ValueItem* funs_TcpNetworkStream_read_available(ValueItem* args, uint32_t len){
	if(len >= 2){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		if(args[1].meta.vtype != VType::raw_arr_ui8 || args[1].meta.vtype != VType::raw_arr_i8)
			throw InvalidArguments("The second argument must be a raw_arr_ui8.");
		return new ValueItem(((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->read_available((char*)args[1].val, args[1].meta.val_len));
	}else
		throw InvalidArguments("The number of arguments must be 2.");
}
ValueItem* funs_TcpNetworkStream_data_available(ValueItem* args, uint32_t len){
	if(len >= 1){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		return new ValueItem(((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->data_available());
	}else
		throw InvalidArguments("The number of arguments must be 1.");
}
ValueItem* funs_TcpNetworkStream_write(ValueItem* args, uint32_t len){
	if(len >= 2){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		if(args[1].meta.vtype != VType::raw_arr_ui8 || args[1].meta.vtype != VType::raw_arr_ui8)
			throw InvalidArguments("The second argument must be a raw_arr_ui8.");
		((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->write((char*)args[1].val, args[1].meta.val_len);
		return nullptr;
	}else
		throw InvalidArguments("The number of arguments must be 2.");
}
ValueItem* funs_TcpNetworkStream_write_file(ValueItem* args, uint32_t len){
	if(len >= 2){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		ValueItem& arg1 = args[1];
		uint64_t data_len = 0;
		uint64_t offset = 0;
		uint32_t chunks_size = 0;
		if(arg1.meta.vtype != VType::proxy && arg1.meta.vtype != VType::string)
			throw InvalidArguments("The second argument must be a file handle or a file path.");
		if(len >= 3)
			data_len = (uint64_t)args[2];
		if(len >= 4)
			offset = (uint64_t)args[3];
		if(len >= 5)
			chunks_size = (uint32_t)args[4];

		if(arg1.meta.vtype == VType::proxy){
			auto& proxy = *((ProxyClass*)args[1].getSourcePtr());
			if(proxy.declare_ty){
				if(proxy.declare_ty->name == "file_handle"){
					return new ValueItem(((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->write_file((*(typed_lgr<::files::FileHandle>*)proxy.class_ptr)->internal_get_handle(), data_len, offset, chunks_size));
				}else if(proxy.declare_ty->name == "blocking_file_handle")
					return new ValueItem(((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->write_file((*(typed_lgr<::files::BlockingFileHandle>*)proxy.class_ptr)->internal_get_handle(), data_len, offset, chunks_size));
			}
			throw InvalidArguments("The second argument must be a file handle or a file path.");
		}
		std::string& path = *((std::string*)arg1.getSourcePtr());
		return new ValueItem(((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->write_file(path.data(), path.size(), data_len, offset, chunks_size));
	}else
		throw InvalidArguments("The number of arguments must be at least 2.");
}
ValueItem* funs_TcpNetworkStream_force_write(ValueItem* args, uint32_t len){
	if(len >= 1){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->force_write();
		return nullptr;
	}else
		throw InvalidArguments("The number of arguments must be 1.");
}
ValueItem* funs_TcpNetworkStream_force_write_and_close(ValueItem* args, uint32_t len){
	if(len >= 2){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		if(args[1].meta.vtype != VType::raw_arr_ui8)
			throw InvalidArguments("The second argument must be a raw_arr_ui8.");
		((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->force_write_and_close((char*)args[1].val, args[1].meta.val_len);
		return nullptr;
	}else
		throw InvalidArguments("The number of arguments must be 2.");
}
ValueItem* funs_TcpNetworkStream_close(ValueItem* args, uint32_t len){
	if(len >= 1){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->close();
		return nullptr;
	}else
		throw InvalidArguments("The number of arguments must be 1.");
}
ValueItem* funs_TcpNetworkStream_reset(ValueItem* args, uint32_t len){
	if(len >= 1){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->reset();
		return nullptr;
	}else
		throw InvalidArguments("The number of arguments must be 1.");
}
ValueItem* funs_TcpNetworkStream_rebuffer(ValueItem* args, uint32_t len){
	if(len >= 2){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->rebuffer((int32_t)args[1]);
		return nullptr;
	}else
		throw InvalidArguments("The number of arguments must be 1.");
}
ValueItem* funs_TcpNetworkStream_is_closed(ValueItem* args, uint32_t len){
	if(len >= 1){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		return new ValueItem(((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->is_closed());
	}else
		throw InvalidArguments("The number of arguments must be 1.");
}
ValueItem* funs_TcpNetworkStream_error(ValueItem* args, uint32_t len){
	if(len >= 1){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		return new ValueItem((uint8_t)((TcpNetworkStream*)((ProxyClass*)args[0].val)->class_ptr)->error());
	}else
		throw InvalidArguments("The number of arguments must be 1.");
}



void init_define_TcpNetworkStream(){
	if(!define_TcpNetworkStream.name.empty())
		return;
	define_TcpNetworkStream.name = "tcp_network_stream";
	define_TcpNetworkStream.copy = nullptr;
	define_TcpNetworkStream.destructor = AttachA::Interface::special::proxyDestruct<TcpNetworkStream, false>;
	define_TcpNetworkStream.funs["read_available_ref"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkStream_read_available_ref, false), false, ClassAccess::pub};
	define_TcpNetworkStream.funs["read_available"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkStream_read_available, false), false, ClassAccess::pub};
	define_TcpNetworkStream.funs["data_available"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkStream_data_available, false), false, ClassAccess::pub};
	define_TcpNetworkStream.funs["write"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkStream_write, false), false, ClassAccess::pub};
	define_TcpNetworkStream.funs["write_file"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkStream_write_file, false), false, ClassAccess::pub};	
	define_TcpNetworkStream.funs["force_write"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkStream_force_write, false), false, ClassAccess::pub};
	define_TcpNetworkStream.funs["force_write_and_close"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkStream_force_write_and_close, false), false, ClassAccess::pub};
	define_TcpNetworkStream.funs["close"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkStream_close, false), false, ClassAccess::pub};
	define_TcpNetworkStream.funs["reset"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkStream_reset, false), false, ClassAccess::pub};
	define_TcpNetworkStream.funs["rebuffer"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkStream_rebuffer, false), false, ClassAccess::pub};
	define_TcpNetworkStream.funs["is_closed"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkStream_is_closed, false), false, ClassAccess::pub};
	define_TcpNetworkStream.funs["error"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkStream_error, false), false, ClassAccess::pub};
}

#pragma endregion

#pragma region TcpNetworkBlocing
class TcpNetworkBlocing{
	friend class TcpNetworkManager;
	tcp_handle* handle;
	TaskMutex mutex;
	tcp_handle::error last_error;
	TcpNetworkBlocing(tcp_handle* handle):handle(handle){}
	bool checkup(){
		if(!handle)
			return false;
		if(!handle->valid()){
			last_error = handle->invalid_reason;
			delete handle;
			handle = nullptr;
			return false;
		}
		return true;
	}
public:
	~TcpNetworkBlocing(){
		std::lock_guard lg(mutex);
		if(handle)
			delete handle;
		handle = nullptr;
	}

	ValueItem read(uint32_t len){
		std::lock_guard lg(mutex);
		if(handle){
			if(!checkup()) return nullptr;
			char* buf = new char[len];
			handle->read_force(len, buf);
			if(len == 0){
				delete[] buf;
				return nullptr;
			}
			return ValueItem((uint8_t*)buf, len, no_copy);
		}
		return nullptr;
	}
	ValueItem available_bytes(){
		std::lock_guard lg(mutex);
		if(handle)
			return handle->available_bytes();
		return 0ui32;
	}
	ValueItem write(char* data, uint32_t len){
		std::lock_guard lg(mutex);
		if(handle){
			if(!checkup()) return nullptr;
			return handle->write_force(data, len);
		}
		return nullptr;
	}
	ValueItem write_file(char* path, size_t len, uint64_t data_len, uint64_t offset, uint32_t block_size){
		std::lock_guard lg(mutex);
		if(handle){
			if(!checkup()) return nullptr;
			return handle->send_file(path, len, data_len, offset, block_size);
		}
		return nullptr;
	}
	ValueItem write_file(void* fhandle, uint64_t data_len, uint64_t offset, uint32_t block_size){
		std::lock_guard lg(mutex);
		if(handle){
			if(!checkup()) return nullptr;
			return handle->send_file(fhandle, data_len, offset, block_size);
		}
		return nullptr;
	}


	void close(){
		std::lock_guard lg(mutex);
		if(handle){
			handle->close();
			last_error = handle->invalid_reason;
			delete handle;
			handle = nullptr;
		}
	}
	void reset(){
		std::lock_guard lg(mutex);
		if(handle){
			handle->reset();
			last_error = handle->invalid_reason;
			delete handle;
			handle = nullptr;
		}
	}
	void rebuffer(size_t new_size){
		std::lock_guard lg(mutex);
		if(handle)
			handle->rebuffer(new_size);
	}
	
	bool is_closed(){
		std::lock_guard lg(mutex);
		if(handle){
			bool res = handle->valid();
			if(!res){
				last_error = handle->invalid_reason;
				delete handle;
				handle = nullptr;
			}
			return !res;
		}
		return true;
	}
	tcp_handle::error error(){
		std::lock_guard lg(mutex);
		if(handle)
			return handle->invalid_reason;
		return last_error;
	}
};

ValueItem* funs_TcpNetworkBlocing_read(ValueItem* args, uint32_t len){
	if(len == 2){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		return new ValueItem(((TcpNetworkBlocing*)((ProxyClass*)args[0].val)->class_ptr)->read((uint32_t)args[1]));
	}else throw InvalidArguments("The first argument must be a client_context.");
}
ValueItem* funs_TcpNetworkBlocing_available_bytes(ValueItem* args, uint32_t len){
	if(len >= 1){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		return new ValueItem(((TcpNetworkBlocing*)((ProxyClass*)args[0].val)->class_ptr)->available_bytes());
	}else throw InvalidArguments("The first argument must be a client_context.");
}
ValueItem* funs_TcpNetworkBlocing_write(ValueItem* args, uint32_t len){
	if(len >= 2){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		if(args[1].meta.vtype != VType::raw_arr_ui8)
			throw InvalidArguments("The third argument must be a raw_arr_ui8.");
		if(len == 2)
			return new ValueItem(((TcpNetworkBlocing*)((ProxyClass*)args[0].val)->class_ptr)->write((char*)args[1].getSourcePtr(), args[1].meta.val_len));
		else{
			uint32_t len = (uint32_t)args[2];
			if(len > args[1].meta.val_len)
				throw OutOfRange("The length of the data to be sent is greater than the length of array.");
			return new ValueItem(((TcpNetworkBlocing*)((ProxyClass*)args[0].val)->class_ptr)->write((char*)args[1].getSourcePtr(), len));
		}
	}else
		throw InvalidArguments("The number of arguments must be at least 2.");
}
ValueItem* funs_TcpNetworkBlocing_write_file(ValueItem* args, uint32_t len){
	if(len >= 2){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		ValueItem& arg1 = args[1];
		uint64_t data_len = 0;
		uint64_t offset = 0;
		uint32_t chunks_size = 0;
		if(arg1.meta.vtype != VType::proxy && arg1.meta.vtype != VType::string)
			throw InvalidArguments("The second argument must be a file handle or a file path.");
		if(len >= 3)
			data_len = (uint64_t)args[2];
		if(len >= 4)
			offset = (uint64_t)args[3];
		if(len >= 5)
			chunks_size = (uint32_t)args[4];

		if(arg1.meta.vtype == VType::proxy){
			auto& proxy = *((ProxyClass*)args[1].val);
			if(proxy.declare_ty){
				if(proxy.declare_ty->name == "file_handle")
					return new ValueItem(((TcpNetworkBlocing*)((ProxyClass*)args[0].val)->class_ptr)->write_file((*(typed_lgr<::files::FileHandle>*)proxy.class_ptr)->internal_get_handle(), data_len, offset, chunks_size));
				else if(proxy.declare_ty->name == "blocking_file_handle")
					return new ValueItem(((TcpNetworkBlocing*)((ProxyClass*)args[0].val)->class_ptr)->write_file((*(typed_lgr<::files::BlockingFileHandle>*)proxy.class_ptr)->internal_get_handle(), data_len, offset, chunks_size));
			}
			throw InvalidArguments("The second argument must be a file handle or a file path.");
		}
		std::string& path = *((std::string*)arg1.getSourcePtr());
		return new ValueItem(((TcpNetworkBlocing*)((ProxyClass*)args[0].val)->class_ptr)->write_file(path.data(), path.size(), data_len, offset, chunks_size));
	}else
		throw InvalidArguments("The number of arguments must be at least 2.");
}
ValueItem* funs_TcpNetworkBlocing_close(ValueItem* args, uint32_t len){
	if(len >= 1){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		((TcpNetworkBlocing*)((ProxyClass*)args[0].val)->class_ptr)->close();
		return nullptr;
	}else throw InvalidArguments("The first argument must be a client_context.");
}
ValueItem* funs_TcpNetworkBlocing_reset(ValueItem* args, uint32_t len){
	if(len >= 1){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		((TcpNetworkBlocing*)((ProxyClass*)args[0].val)->class_ptr)->reset();
		return nullptr;
	}else throw InvalidArguments("The first argument must be a client_context.");
}
ValueItem* funs_TcpNetworkBlocing_rebuffer(ValueItem* args, uint32_t len){
	if(len >= 2){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		((TcpNetworkBlocing*)((ProxyClass*)args[0].val)->class_ptr)->rebuffer((int32_t)args[1]);
		return nullptr;
	}else throw InvalidArguments("The first argument must be a client_context.");
}
ValueItem* funs_TcpNetworkBlocing_is_closed(ValueItem* args, uint32_t len){
	if(len >= 1){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		return new ValueItem(((TcpNetworkBlocing*)((ProxyClass*)args[0].val)->class_ptr)->is_closed());
	}else throw InvalidArguments("The first argument must be a client_context.");
}
ValueItem* funs_TcpNetworkBlocing_error(ValueItem* args, uint32_t len){
	if(len >= 1){
		if(args[0].meta.vtype != VType::proxy)
			throw InvalidArguments("The first argument must be a client_context.");
		return new ValueItem((uint8_t)((TcpNetworkBlocing*)((ProxyClass*)args[0].val)->class_ptr)->error());
	}else throw InvalidArguments("The first argument must be a client_context.");
}

void init_define_TcpNetworkBlocking(){
	if(!define_TcpNetworkBlocking.name.empty())
		return;
	define_TcpNetworkBlocking.name = "tcp_network_blocking";
	define_TcpNetworkBlocking.destructor = AttachA::Interface::special::proxyDestruct<TcpNetworkBlocing, false>;
	define_TcpNetworkBlocking.copy = nullptr;
	define_TcpNetworkBlocking.funs["read"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkBlocing_read, false), false, ClassAccess::pub};
	define_TcpNetworkBlocking.funs["available_bytes"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkBlocing_available_bytes, false), false, ClassAccess::pub};
	define_TcpNetworkBlocking.funs["write"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkBlocing_write, false), false, ClassAccess::pub};
	define_TcpNetworkBlocking.funs["write_file"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkBlocing_write_file, false), false, ClassAccess::pub};
	define_TcpNetworkBlocking.funs["close"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkBlocing_close, false), false, ClassAccess::pub};
	define_TcpNetworkBlocking.funs["reset"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkBlocing_reset, false), false, ClassAccess::pub};
	define_TcpNetworkBlocking.funs["rebuffer"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkBlocing_rebuffer, false), false, ClassAccess::pub};
	define_TcpNetworkBlocking.funs["is_closed"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkBlocing_is_closed, false), false, ClassAccess::pub};
	define_TcpNetworkBlocking.funs["error"] = ClassFnDefine{new FuncEnviropment(funs_TcpNetworkBlocing_error, false), false, ClassAccess::pub};
}

#pragma endregion

class TcpNetworkManager : public NativeWorkerManager {
	TaskMutex safety;
	typed_lgr<class FuncEnviropment> handler_fn;
	typed_lgr<class FuncEnviropment> accept_filter;
    sockaddr_in6 connectionAddress;
	SOCKET main_socket;
public:
	int32_t default_len;
private:
	bool allow_new_connections = false;
	bool disabled = true;
	bool corrupted = false;
	size_t acceptors;
	TcpNetworkServer::ManageType manage_type;
	TaskConditionVariable state_changed_cv;

	void make_acceptEx(void){
	re_try:
		static const auto adress_len = sizeof(sockaddr_storage) + 16;
		auto new_sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		tcp_handle *pClientContext = new tcp_handle(new_sock, default_len, this);
		pClientContext->opcode = tcp_handle::Opcode::ACCEPT;
		BOOL success = _AcceptEx(
			main_socket,
			new_sock,
			pClientContext->buffer.buf,
			0,
			adress_len,
			adress_len,
			NULL,
			&pClientContext->overlapped
		);
		if(success != TRUE) {
			auto err = WSAGetLastError();
			if (err == WSA_IO_PENDING) 
				return;
			else if (err == WSAECONNRESET) {
				delete pClientContext;
				goto re_try;
			}
			else {
				delete pClientContext;
				return;
			}
		}
	}
	ValueItem accept_manager_construct(tcp_handle* self){
		switch (manage_type) {
		case TcpNetworkServer::ManageType::blocking:
			return ValueItem(new ProxyClass(new TcpNetworkBlocing(self), &define_TcpNetworkBlocking), VType::proxy, no_copy);
		case TcpNetworkServer::ManageType::write_delayed:
			return ValueItem(new ProxyClass(new TcpNetworkStream(self), &define_TcpNetworkStream), VType::proxy, no_copy);
		default:
			return nullptr;
		}
	}
	void accepted(tcp_handle* self,ValueItem clientAddr, ValueItem localAddr){
		if(!allow_new_connections){
			delete self;
			return;
		}
		std::lock_guard guard(safety);
		Task::start(new Task(handler_fn, ValueItem{
			accept_manager_construct(self), 
			std::move(clientAddr),
			std::move(localAddr)
		}));
	}


	void new_connection(tcp_handle& data){
		make_acceptEx();
		
		universal_address* pClientAddr = NULL;
        universal_address* pLocalAddr = NULL;
        int remoteLen = sizeof(universal_address);
        int localLen = sizeof(universal_address);
        _GetAcceptExSockaddrs(data.buffer.buf,
                                     0,
                                     sizeof(universal_address) + 16,
                                     sizeof(universal_address) + 16,
                                     (LPSOCKADDR*)&pLocalAddr,
                                     &localLen,
                                     (LPSOCKADDR*)&pClientAddr,
                                     &remoteLen);
		ValueItem clientAddress(new ProxyClass(new universal_address(*pClientAddr),&define_UniversalAddress), VType::proxy, no_copy);
		ValueItem localAddress(new ProxyClass(new universal_address(*pLocalAddr),&define_UniversalAddress), VType::proxy, no_copy);
		if(accept_filter){
			if(AttachA::cxxCall(accept_filter,clientAddress,localAddress)){
				closesocket(data.socket);
				#ifndef DISABLE_RUNTIME_INFO
				auto tmp = UniversalAddress::_define_to_string(&clientAddress,1);
				ValueItem notify{ "Client: " + (std::string)*tmp + " not accepted due filter" };
				delete tmp;
				info.async_notify(notify);
				#endif
				delete &data;
				return;
			}
		}
		
		#ifndef DISABLE_RUNTIME_INFO
		{
			auto tmp = UniversalAddress::_define_to_string(&clientAddress,1);
			ValueItem notify{ "Client connected from: " + (std::string)*tmp };
			delete tmp;
			info.async_notify(notify);
		}
		#endif
		setsockopt(data.socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&main_socket, sizeof(main_socket) );
		{
			std::lock_guard lock(safety);
			if(!NativeWorkersSingleton::register_handle((HANDLE)data.socket, &data)){
				closesocket(data.socket);
				#ifndef DISABLE_RUNTIME_INFO
				auto tmp = UniversalAddress::_define_to_string(&clientAddress,1);
				ValueItem notify{ "Client: " + (std::string)*tmp + " not accepted because register handle failed " + std::to_string(data.socket) };
				delete tmp;
				info.sync_notify(notify);
				#endif
				delete &data;
				return;
			}
		}
		
		accepted(&data, std::move(clientAddress), std::move(localAddress));
		return;
	}
public:
	TcpNetworkManager(universal_address& ip_port, size_t acceptors,TcpNetworkServer::ManageType manage_type, int32_t timeout_ms, int32_t default_buffer) : acceptors(acceptors),manage_type(manage_type), default_len(default_buffer) {
    	memcpy(&connectionAddress, &ip_port, sizeof(sockaddr_in6));
		main_socket = WSASocketW(AF_INET6, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (main_socket == INVALID_SOCKET){
			ValueItem error = std::string("Failed create socket: ") + std::to_string(WSAGetLastError());
			errors.sync_notify(error);
			corrupted = true;
			return;
		}
		DWORD argp = 1;//non blocking
		int result = setsockopt(main_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&argp, sizeof(argp));
		if (result == SOCKET_ERROR){
			ValueItem error = std::string("Failed set reuse addr: ") + std::to_string(WSAGetLastError());
			errors.sync_notify(error);
			corrupted = true;
			return;
		}
		//ignored error, it is not critical
		result = setsockopt(main_socket,IPPROTO_TCP,TCP_FASTOPEN,(char*)&argp, sizeof(argp));
		if (result == SOCKET_ERROR){
			ValueItem warn = std::string("Failed enable fast open for server(") + std::to_string(WSAGetLastError()) + "), continue slow mode";
			warning.async_notify(warn);
		}
		argp = 0;
		result = setsockopt(main_socket,IPPROTO_IPV6,IPV6_V6ONLY,(char*)&argp, sizeof(argp));
		if (result == SOCKET_ERROR){
			ValueItem error = std::string("Failed set dual mode: ") + std::to_string(WSAGetLastError());
			errors.sync_notify(error);
			corrupted = true;
			return;
		}
		
		if (ioctlsocket(main_socket, FIONBIO, &argp) == SOCKET_ERROR){
			ValueItem error = std::string("Failed set no block mode: ") + std::to_string(WSAGetLastError());
			errors.sync_notify(error);
			corrupted = true;
			return;
		}
		//set timeout
		DWORD timeout = timeout_ms;
		if (setsockopt(main_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR){
			ValueItem error = std::string("Failed set recv timeout: ") + std::to_string(WSAGetLastError());
			errors.sync_notify(error);
			corrupted = true;
			return;
		}

		init_win_fns(main_socket);
		if (bind(main_socket, (sockaddr*)&connectionAddress, sizeof(sockaddr_in6)) == SOCKET_ERROR){
			ValueItem error = std::string("Failed bind: ") + std::to_string(WSAGetLastError());
			errors.sync_notify(error);
			corrupted = true;
			return;
		}
		if(!NativeWorkersSingleton::register_handle((HANDLE)main_socket, this)){
			ValueItem error = std::string("Failed register handle: ") + std::to_string(GetLastError());
			errors.sync_notify(error);
			corrupted = true;
		}
	}
	~TcpNetworkManager(){
		shutdown();
	}
	
	void handle(void* _data, NativeWorkerHandle* overlapped, unsigned long dwBytesTransferred, bool status) override {
		auto& data = *(tcp_handle*)overlapped;
		if(data.opcode == tcp_handle::Opcode::ACCEPT) new_connection(data);
		else if (!((FALSE == status) || ((true == status) && (0 == dwBytesTransferred)))) data.handle(dwBytesTransferred);
		else {
			#ifndef DISABLE_RUNTIME_INFO
			{
				ValueItem notify{ "Client disconnected (client hash: " + std::to_string(std::hash<void*>()(overlapped))+')' };
				info.async_notify(notify);
			}
			#endif
			data.connection_reset();
		}
	}
	void set_on_connect(typed_lgr<class FuncEnviropment> handler_fn, TcpNetworkServer::ManageType manage_type){
		if(corrupted)
			throw AttachARuntimeException("TcpNetworkManager is corrupted");
		std::lock_guard lock(safety);
		this->handler_fn = handler_fn;
		this->manage_type = manage_type;
	}
	void shutdown(){
		if(corrupted)
			throw AttachARuntimeException("TcpNetworkManager is corrupted");
		std::lock_guard lock(safety);
		if(disabled)
			return;
		if(closesocket(main_socket) == SOCKET_ERROR)
			WSACleanup();
		allow_new_connections = false;
		disabled = true;
		state_changed_cv.notify_all();
	}
	void pause(){
		if(corrupted)
			throw AttachARuntimeException("TcpNetworkManager is corrupted");
		allow_new_connections = false;
	}
	void resume(){
		if(corrupted)
			throw AttachARuntimeException("TcpNetworkManager is corrupted");
		allow_new_connections = true;
	}
	void start(){
		if(corrupted)
			throw AttachARuntimeException("TcpNetworkManager is corrupted");
		std::lock_guard lock(safety);
		allow_new_connections = true; 
		if(!disabled)
			return;
		if (listen(main_socket, SOMAXCONN) == SOCKET_ERROR){
			WSACleanup();
			return;
		}
		for(size_t i = 0; i < acceptors; i++)
			make_acceptEx();
		disabled = false;
		state_changed_cv.notify_all();
	}
	void _await(){
		MutexUnify um(safety);
		std::unique_lock lock(um);
		if(corrupted)
			throw AttachARuntimeException("TcpNetworkManager is corrupted");
		while(!disabled)
			state_changed_cv.wait(lock);
	}


	void set_accept_filter(typed_lgr<class FuncEnviropment> filter){
		if(corrupted)
			throw AttachARuntimeException("TcpNetworkManager is corrupted");
		std::lock_guard lock(safety);
		this->accept_filter = filter;
	}
	bool is_corrupted(){
		return corrupted;
	}

	uint16_t port(){
		if(corrupted)
			throw AttachARuntimeException("TcpNetworkManager is corrupted");
		return htons(connectionAddress.sin6_port);
	}
	std::string ip(){
		if(corrupted)
			throw AttachARuntimeException("TcpNetworkManager is corrupted");

		ProxyClass tmp(&connectionAddress, &define_UniversalAddress);
		ValueItem args(&tmp, VType::proxy, as_refrence);
		ValueItem* res;
		try{
			res = UniversalAddress::_define_to_string(&args, 1);
		}catch(...){
			tmp.declare_ty = nullptr;//prevent delete
			throw;
		}
		tmp.declare_ty = nullptr;//prevent delete
		std::string ret = (std::string)*res;
		delete res;
		return ret;
	}
	ValueItem address(){
		if(corrupted)
			throw AttachARuntimeException("TcpNetworkManager is corrupted");

		sockaddr_storage* addr = new sockaddr_storage;
		memcpy(addr, &connectionAddress, sizeof(sockaddr_in6));
		memset(addr + sizeof(sockaddr_in6), 0, sizeof(sockaddr_storage) - sizeof(sockaddr_in6));
		return ValueItem(new ProxyClass(addr, &define_UniversalAddress), VType::proxy, no_copy);
	}

	bool is_paused(){
		return !disabled && !allow_new_connections;
	}
	bool in_run(){
		return !disabled;
	}
};
class TcpClientManager : public NativeWorkerManager {
	TaskMutex mutex;
    sockaddr_in6 connectionAddress;
	tcp_handle* _handle;
	bool corrupted = false;
public:
	void handle(void* _data, NativeWorkerHandle* overlapped, unsigned long dwBytesTransferred, bool status) override {
		tcp_handle& handle = *(tcp_handle*)overlapped;
		if((FALSE == status) || ((true == status) && (0 == dwBytesTransferred)))
			handle.connection_reset();
		else if(handle.opcode == tcp_handle::Opcode::ACCEPT)
			Task::start(handle.notify_task);
		else
			handle.handle(dwBytesTransferred);
	}

	TcpClientManager(sockaddr_in6& _connectionAddress, int32_t timeout_ms = 0) : connectionAddress(_connectionAddress) {
		if(timeout_ms < 0) timeout_ms = 0;
		SOCKET clientSocket = WSASocketW(AF_INET6, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (clientSocket == INVALID_SOCKET) {
			corrupted = true;
			return;
		}
		DWORD argp = 1;
		if (ioctlsocket(clientSocket, FIONBIO, &argp) == SOCKET_ERROR){
			ValueItem error = std::string("Failed set no block mode: ") + std::to_string(WSAGetLastError());
			errors.sync_notify(error);
			corrupted = true;
			return;
		}
		_handle = new tcp_handle(clientSocket,4096, this);
		typed_lgr<Task> task_await = _handle->notify_task = Task::dummy_task();
		if(!_ConnectEx(clientSocket, (sockaddr*)&connectionAddress, sizeof(connectionAddress), NULL, 0, NULL, (OVERLAPPED*)_handle)){
			auto err = WSAGetLastError();
			if(err != ERROR_IO_PENDING){
				corrupted = true;
				return;
			}
		}
		Task::await_task(task_await);
		_handle->notify_task = nullptr;
	}
	TcpClientManager(sockaddr_in6& _connectionAddress, char* data, uint32_t len, int32_t timeout_ms = 0) : connectionAddress(_connectionAddress) {
		if(timeout_ms < 0) timeout_ms = 0;
		SOCKET clientSocket = WSASocketW(AF_INET6, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (clientSocket == INVALID_SOCKET) {
			corrupted = true;
			return;
		}
		DWORD argp = 1;
		if (setsockopt(clientSocket,IPPROTO_TCP,TCP_FASTOPEN,(char*)&argp, sizeof(argp))){
			ValueItem warn = std::string("Failed enable fast open for client(") + std::to_string(WSAGetLastError()) + "), continue slow mode";
			warning.async_notify(warn);
		}
		if (ioctlsocket(clientSocket, FIONBIO, &argp) == SOCKET_ERROR){
			ValueItem error = std::string("Failed set no block mode: ") + std::to_string(WSAGetLastError());
			errors.sync_notify(error);
			corrupted = true;
			return;
		}
		_handle = new tcp_handle(clientSocket,4096, this);
		char* old_buffer = _handle->data;
		_handle->data = data;
		_handle->buffer.buf = data;
		_handle->buffer.len = len;
		_handle->total_bytes = len;
		_handle->opcode = tcp_handle::Opcode::WRITE;
		typed_lgr<Task> task_await = _handle->notify_task = Task::dummy_task();
		if(!_ConnectEx(clientSocket, (sockaddr*)&connectionAddress, sizeof(connectionAddress), data, len, NULL, (OVERLAPPED*)_handle)){
			auto err = WSAGetLastError();
			if(err != ERROR_IO_PENDING){
				corrupted = true;
				return;
			}
		}
		Task::await_task(task_await);
		_handle->data = old_buffer;
		_handle->notify_task = nullptr;

	}
	~TcpClientManager() override {
		if(corrupted)
			return;
		delete _handle;
	}

	int32_t read(char* data, int32_t len){
		if(corrupted)
			throw std::runtime_error("TcpClientManager::read, corrupted");
		std::lock_guard<TaskMutex> lock(mutex);
		int32_t readed = 0; 
		while(!_handle->available_bytes())
			if(!_handle->send_queue_item()) break;
		_handle->read_available(data, len, readed);
		return readed;
	}
	bool write(const char* data, int32_t len){
		if(corrupted)
			throw std::runtime_error("TcpClientManager::write, corrupted");
		std::lock_guard<TaskMutex> lock(mutex);
		_handle->send_data(data, len);
		while(!_handle->available_bytes())
			if(!_handle->send_queue_item())break;
		return _handle->valid();
	}
	bool write_file(const char* path, size_t len, uint64_t data_len, uint64_t offset, uint32_t chunks_size){
		if(corrupted)
			throw std::runtime_error("TcpClientManager::write_file, corrupted");
		std::lock_guard<TaskMutex> lock(mutex);
		while(!_handle->available_bytes())
			if(!_handle->send_queue_item())break;
		return _handle->send_file(path, len, data_len, offset, chunks_size);
	}
	bool write_file(void* handle, uint64_t data_len, uint64_t offset, uint32_t chunks_size){
		if(corrupted)
			throw std::runtime_error("TcpClientManager::write_file, corrupted");
		std::lock_guard<TaskMutex> lock(mutex);
		while(!_handle->available_bytes())
			if(!_handle->send_queue_item())break;
		return _handle->send_file(handle, data_len, offset, chunks_size);
	}
	void close(){
		if(corrupted)
			throw std::runtime_error("TcpClientManager::close, corrupted");
		std::lock_guard<TaskMutex> lock(mutex);
		_handle->close();
	}
	void reset(){
		if(corrupted)
			throw std::runtime_error("TcpClientManager::close, corrupted");
		std::lock_guard<TaskMutex> lock(mutex);
		_handle->reset();
	}
	bool is_corrupted(){
		return corrupted;
	}
	void rebuffer(uint32_t size){
		if(corrupted)
			throw std::runtime_error("TcpClientManager::rebuffer, corrupted");
		std::lock_guard<TaskMutex> lock(mutex);
		_handle->rebuffer(size);
	}
};
#pragma endregion

class udp_handle : public NativeWorkerHandle, public NativeWorkerManager {
	typed_lgr<Task> notify_task;
	SOCKET socket;
	sockaddr_in6 server_address;
public:
	DWORD fullifed_bytes;
	bool status;
	DWORD last_error;
	udp_handle(sockaddr_in6& address, uint32_t timeout_ms) : NativeWorkerHandle(this){
		socket = WSASocketW(AF_INET6, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if(socket == INVALID_SOCKET)
			return;
		if(bind(socket, (sockaddr*)&address, sizeof(sockaddr_in6)) == SOCKET_ERROR){
			closesocket(socket);
			socket = INVALID_SOCKET;
			return;
		}
		server_address = address;
	}
	void handle(void* data, NativeWorkerHandle* overlapped, unsigned long fullifed_bytes, bool status) override{
		this->fullifed_bytes = fullifed_bytes;
		this->status = status;
		last_error = GetLastError();
		Task::start(notify_task);
	}
	void recv(uint8_t* data, uint32_t size, sockaddr_storage& sender, int& sender_len){
		if(socket == INVALID_SOCKET)
			throw InvalidOperation("Socket not connected");
		WSABUF buf;
		buf.buf = (char*)data;
		buf.len = size;
		notify_task = Task::dummy_task();
		if(WSARecvFrom(socket, &buf, 1, nullptr, 0, (sockaddr*)&sender, &sender_len, (OVERLAPPED*)this, nullptr)){
			if(WSAGetLastError() != WSA_IO_PENDING){
				last_error = WSAGetLastError();
				status = false;
				fullifed_bytes = 0;
				notify_task = nullptr;
				return;
			}
		}
		Task::await_task(notify_task);
		notify_task = nullptr;
	}
	void send(uint8_t* data, uint32_t size, sockaddr_storage& to){
		sockaddr_in6 sender;
		WSABUF buf;
		buf.buf = (char*)data;
		buf.len = size;
		notify_task = Task::dummy_task();
		if(WSASendTo(socket, &buf, 1, nullptr, 0, (sockaddr*)&to, sizeof(to), (OVERLAPPED*)this, nullptr)){
			if(WSAGetLastError() != WSA_IO_PENDING){
				last_error = WSAGetLastError();
				status = false;
				fullifed_bytes = 0;
				notify_task = nullptr;
				return;
			}
		}
		Task::await_task(notify_task);
		notify_task = nullptr;
	}
};

uint8_t init_networking(){
	init_define_UniversalAddress();
	init_define_TcpNetworkStream();
	init_define_TcpNetworkBlocking();
	
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)){
		auto err = WSAGetLastError();
		switch (err) {
		case WSASYSNOTREADY:return 1;
		case WSAVERNOTSUPPORTED:return 2;return 3;
		case WSAEPROCLIM:return 4;
		case WSAEFAULT:return 5;
		default:return 0xFF;
		}
	};
	inited = true;
	return 0;
}
void deinit_networking(){
	if(inited)
		WSACleanup();
	inited = false;
}

#else
//TO_DO: LINUX implementation using io_uring

uint8_t init_networking(){
	init_define_UniversalAddress();
	init_define_TcpNetworkStream();
	init_define_TcpNetworkBlocking();
	
	inited = true;
}
void deinit_networking(){
	//inited = false;
}
#endif

TcpNetworkServer::TcpNetworkServer(typed_lgr<class FuncEnviropment> on_connect, ValueItem& ip_port, ManageType mt, size_t acceptors, int32_t timeout_ms, int32_t default_buffer){
	if(!inited)
		throw InternalException("Network module not initialized");
	sockaddr_storage address;
	get_address_from_valueItem(ip_port, address);
	timeout_ms = timeout_ms < 0 ? 0 : timeout_ms;
	handle = new TcpNetworkManager(address, acceptors, mt, timeout_ms, default_buffer);
	handle->set_on_connect(on_connect, mt);
}
TcpNetworkServer::~TcpNetworkServer(){
	if(handle)
		delete handle;
	handle = nullptr;
}
void TcpNetworkServer::start(){
	handle->start();
}
void TcpNetworkServer::pause(){
	handle->pause();
}
void TcpNetworkServer::resume(){
	handle->resume();
}
void TcpNetworkServer::stop(){
	handle->shutdown();
}
bool TcpNetworkServer::is_running(){
	return handle->in_run();
}
void TcpNetworkServer::_await(){
	handle->_await();
}
bool TcpNetworkServer::is_corrupted(){
	return handle->is_corrupted();
}

uint16_t TcpNetworkServer::server_port(){
	return handle->port();
}
std::string TcpNetworkServer::server_ip(){
	return handle->ip();
}
ValueItem TcpNetworkServer::server_address(){
	return handle->address();
}
bool TcpNetworkServer::is_paused(){
	return handle->is_paused();
}
void TcpNetworkServer::set_default_buffer_size(int32_t size){
	if(handle){
		if(size < 0)
			throw InvalidArguments("Buffer size must be positive");
		handle->default_len = size;
	}
}
void TcpNetworkServer::set_accept_filter(typed_lgr<class FuncEnviropment> filter){
	if(handle)
		handle->set_accept_filter(filter);
}

TcpClientSocket::TcpClientSocket(){}
TcpClientSocket::~TcpClientSocket(){
	if(handle)
		delete handle;
	handle = nullptr;
}
TcpClientSocket* TcpClientSocket::connect(ValueItem& ip_port){
	return connect(ip_port, 0);
}
TcpClientSocket* TcpClientSocket::connect(ValueItem& ip_port, int32_t timeout_ms){
	if(!inited)
		throw InternalException("Network module not initialized");
	sockaddr_storage address;
	get_address_from_valueItem(ip_port, address);
	std::unique_ptr<TcpClientSocket> result;
	result.reset(new TcpClientSocket());
	result->handle = new TcpClientManager((sockaddr_in6&)address, timeout_ms);
	return result.release();
}

TcpClientSocket* TcpClientSocket::connect(ValueItem& ip_port, char* data, uint32_t size){
	return connect(ip_port, data, size, 0);
}
TcpClientSocket* TcpClientSocket::connect(ValueItem& ip_port, char* data, uint32_t size, int32_t timeout_ms){
	if(!inited)
		throw InternalException("Network module not initialized");
	sockaddr_storage address;
	get_address_from_valueItem(ip_port, address);
	std::unique_ptr<TcpClientSocket> result;
	result.reset(new TcpClientSocket());
	result->handle = new TcpClientManager((sockaddr_in6&)address, data, size, timeout_ms);
	return result.release();
}
int32_t TcpClientSocket::recv(uint8_t* data, int32_t size){
	if(!inited)
		throw InternalException("Network module not initialized");
		
	if(handle)
		return handle->read((char*)data, size);
	return 0;
}
bool TcpClientSocket::send(uint8_t* data, int32_t size){
	if(!inited)
		throw InternalException("Network module not initialized");
	if(handle)
		return handle->write((char*)data, size);
	return false;
}
bool TcpClientSocket::send_file(const char* file_path, size_t file_path_len, uint64_t data_len, uint64_t offset, uint32_t chunks_size){
	if(!handle)
		throw InvalidOperation("Socket not connected");
	return handle->write_file(file_path, file_path_len, data_len, offset, chunks_size);
}
bool TcpClientSocket::send_file(class ::files::FileHandle& file, uint64_t data_len, uint64_t offset, uint32_t chunks_size){
	if(!handle)
		throw InvalidOperation("Socket not connected");
	return handle->write_file(file.internal_get_handle(), data_len, offset, chunks_size);
}
bool TcpClientSocket::send_file(class ::files::BlockingFileHandle& file, uint64_t data_len, uint64_t offset, uint32_t chunks_size){
	if(!handle)
		throw InvalidOperation("Socket not connected");
	return handle->write_file(file.internal_get_handle(), data_len, offset, chunks_size);
}
void TcpClientSocket::close(){
	if(handle){
		handle->close();
		delete handle;
		handle = nullptr;
	}
}
void TcpClientSocket::reset(){
	if(handle){
		handle->reset();
		delete handle;
		handle = nullptr;
	}
}
void TcpClientSocket::rebuffer(int32_t size){
	if(handle)
		handle->rebuffer(size);
}


udp_socket::udp_socket(ValueItem& ip_port, uint32_t timeout_ms){
	sockaddr_storage address;
	get_address_from_valueItem(ip_port, address);
	handle = new udp_handle((sockaddr_in6&)address, timeout_ms);
}
udp_socket::~udp_socket(){
	if(handle)
		delete handle;
}

uint32_t udp_socket::recv(uint8_t* data, uint32_t size, ValueItem& sender){
	sockaddr_storage sender_address;
	int sender_len = sizeof(sender_address);
	handle->recv(data, size, sender_address, sender_len);
	if(!handle->status || handle->fullifed_bytes < 0)
		throw AttachARuntimeException("Error while receiving data from udp socket with error code: " + std::to_string(handle->last_error));
	sender = ValueItem(new ProxyClass(new  sockaddr_storage(sender_address), &define_UniversalAddress), VType::proxy);
	return handle->fullifed_bytes;
}
uint32_t udp_socket::send(uint8_t* data, uint32_t size, ValueItem& to){
	sockaddr_storage to_ip_port;
	get_address_from_valueItem(to, to_ip_port);
	handle->send(data, size, to_ip_port);
	if(!handle->status || handle->fullifed_bytes < 0)
		throw AttachARuntimeException("Error while receiving data from udp socket with error code: " + std::to_string(handle->last_error));
	return handle->fullifed_bytes;
}






bool ipv6_supported(){
	if(!inited)
		throw InternalException("Network module not initialized");
	static int ipv6_supported = -1;
	if(ipv6_supported == -1){
		ipv6_supported = 0;
		SOCKET sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
		if(sock != INVALID_SOCKET){
			ipv6_supported = 1;
			closesocket(sock);
		}
	}
	return ipv6_supported == 1;
}




ValueItem makeIP4(const char* ip, uint16_t port){
	if(!inited)
		throw InternalException("Network module not initialized");
	universal_address* addr_storage = new universal_address;
	internal_makeIP4(*addr_storage, ip, port);
	return ValueItem(new ProxyClass(addr_storage, &define_UniversalAddress), VType::proxy);
}
ValueItem makeIP6(const char* ip, uint16_t port){
	if(!inited)
		throw InternalException("Network module not initialized");
	universal_address* addr_storage = new universal_address;
	internal_makeIP6(*addr_storage, ip, port);
	return ValueItem(new ProxyClass(addr_storage, &define_UniversalAddress), VType::proxy);
}
ValueItem makeIP(const char* ip, uint16_t port){
	if(!inited)
		throw InternalException("Network module not initialized");
	universal_address* addr_storage = new universal_address;
	internal_makeIP(*addr_storage, ip, port);
	return ValueItem(new ProxyClass(addr_storage, &define_UniversalAddress), VType::proxy);
}


ValueItem makeIP4_port(const char* ip_port){
	if(!inited)
		throw InternalException("Network module not initialized");
	universal_address* addr_storage = new universal_address;
	internal_makeIP4_port(*addr_storage, ip_port);
	return ValueItem(new ProxyClass(addr_storage, &define_UniversalAddress), VType::proxy);
}
ValueItem makeIP6_port(const char* ip_port){
	if(!inited)
		throw InternalException("Network module not initialized");
	universal_address* addr_storage = new universal_address;
	internal_makeIP6_port(*addr_storage, ip_port);
	return ValueItem(new ProxyClass(addr_storage, &define_UniversalAddress), VType::proxy);
}
ValueItem makeIP_port(const char* ip_port){
	if(ip_port[0] == '[')
		return makeIP6_port(ip_port);
	else
		return makeIP4_port(ip_port);
}
