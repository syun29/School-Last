#include "CMultiThread.h"

CLoadThread *CLoadThread::m_instance;
CLoadThread::CLoadThread() : m_resourceLoader(nullptr), m_loaded(false)
{

	if(m_resourceLoader ) delete m_resourceLoader;
	m_resourceLoader = nullptr;
}

CLoadThread::~CLoadThread()
{
	if (m_resourceLoader)
	{
		m_resourceLoader->join();
		delete m_resourceLoader;
	}
	m_resourceLoader = nullptr;
}

void CLoadThread::LoadThread(std::function<void()> loadProc) {

	loadProc();
	m_loaded = true;
}
void CLoadThread::LoadStart(std::function<void()> loadProc) {
	m_loaded = false;
	if (m_resourceLoader) {
		m_resourceLoader->join();
		delete m_resourceLoader;
		m_resourceLoader = nullptr;
	}


	m_resourceLoader = new  std::thread(std::bind(&CLoadThread::LoadThread, this, loadProc));



}
bool CLoadThread::CheckEnd() {
	return (!m_resourceLoader || m_loaded);
}