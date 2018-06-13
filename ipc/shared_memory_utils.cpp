#include <boost/date_time/posix_time/posix_time.hpp>
#include "shared_memory_utils.h"

using namespace boost::interprocess;

#define SHARED_NAME  "video_shared_lock"

SharedMemoryUtils::SharedMemoryUtils() :
	m_shm(NULL),
	m_mapped_region(NULL) {



}

SharedMemoryUtils::~SharedMemoryUtils() {

}

void SharedMemoryUtils::deleteSharedMemory() {
	shared_memory_object::remove(SHARED_NAME);
}

bool SharedMemoryUtils::openSharedMemory() {

	m_shm = boost::shared_ptr<shared_memory_object>(
		new shared_memory_object(open_only, SHARED_NAME, read_write));

	m_mapped_region = boost::shared_ptr<mapped_region>(new mapped_region(*m_shm, read_write));

	m_data = static_cast<shared_data*>(m_mapped_region->get_address());

	return true;
}


bool SharedMemoryUtils::createSharedMemory() {

	try {
		m_shm = boost::shared_ptr<shared_memory_object>(
			new shared_memory_object(create_only, SHARED_NAME, read_write));
	}
	catch (interprocess_exception &e) {
		m_shm = boost::shared_ptr<shared_memory_object>(
			new shared_memory_object(open_only, SHARED_NAME, read_write));
	}
	

	m_shm->truncate(sizeof(shared_data));

	m_mapped_region = boost::shared_ptr<mapped_region>(new mapped_region(*m_shm, read_write));

	m_data = static_cast<shared_data*>(m_mapped_region->get_address());

	return true;

}


void SharedMemoryUtils::Send(HANDLE handle) {
	if (this->m_data) {

		//m_file_lock->lock();

		this->m_data->handle = handle;

		//m_file_lock->unlock();

		m_data->m_semaphore.post();
	}
}


bool SharedMemoryUtils::Get(HANDLE &handle) {
	if (this->m_data) {
		auto timeout = !m_data->m_semaphore.timed_wait(boost::posix_time::ptime(
			boost::posix_time::microsec_clock::universal_time() + boost::posix_time::milliseconds(5000)));



		if (timeout)
			return false;

		//m_file_lock->lock();


		handle = this->m_data->handle;

		//m_file_lock->unlock();

		return true;

	}

	return false;
}