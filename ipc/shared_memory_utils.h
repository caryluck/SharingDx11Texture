#pragma once
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <wtypes.h>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

using namespace boost::interprocess;

struct shared_data {
	HANDLE handle = 0;
	boost::interprocess::interprocess_semaphore m_semaphore = 0;
};



class SharedMemoryUtils {
public:
	SharedMemoryUtils();

	virtual ~SharedMemoryUtils();

	void Send(HANDLE handle);

	bool Get(HANDLE &handle);

	bool openSharedMemory();

	bool createSharedMemory();

	void deleteSharedMemory();


private:
	
	
	shared_data *m_data = 0;
	
	boost::shared_ptr<file_lock> m_file_lock;

	boost::shared_ptr<shared_memory_object> m_shm;
	boost::shared_ptr<mapped_region> m_mapped_region;
	

};


