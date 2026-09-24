#include "CResource.h"
#include "GL.h"
#include <assert.h>

CResourceManager*	CResourceManager::m_instance = nullptr;

CResourceManager* CResourceManager::GetInstance()
{
	if (!m_instance) m_instance = new CResourceManager;
	return m_instance;
}
void CResourceManager::ClearInstance()
{
	if (!m_instance) return;
	delete m_instance;
	m_instance = nullptr;
}
CResourceManager::CResourceManager() {

}
CResourceManager::~CResourceManager() {
	for (auto it = m_list.begin(); it != m_list.end(); it++) {
		for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
			if (it2->second) {
				it2->second->Release();
				delete it2->second;
			}
			it2->second = nullptr;
		}
		it->second.clear();
	}
	m_list.clear();
}

void CResourceManager::Add(CResource* res,int group) {
	if (!res) return;
	m_list[group][res->m_name] = res;
}
void CResourceManager::Add(std::string name,CResource* res, int group) {
	if (!res) return;
	memcpy(res->m_name,name.c_str(), name.length());
	m_list[group][res->m_name] = res;
}
CResource* CResourceManager::Get(std::string name, int group) {
	CResource* r = m_list[group][name];
	if (r == nullptr) {
		MessageBox(GL::hWnd, (name + "‚ª‚ ‚è‚Ü‚¹‚ñ").c_str(), "", MB_OK);
	}
	return r;
}
bool CResourceManager::IsSet(std::string name, int group) {
	return m_list[group][name]!=nullptr;
}
void CResourceManager::Delete(std::string name,int group) {
	CResource* tmp = m_list[group][name];
	if (tmp) {
		tmp->Release();
		delete tmp;
		m_list[group][name] = nullptr;
	}
}
void CResourceManager::Delete(int group) {
	for (auto it = m_list[group].begin(); it != m_list[group].end(); it++) {
		if (it->second) {
			it->second->Release();
			delete it->second;
		}
		it->second = nullptr;
	}
	m_list[group].clear();
}
unsigned int CResourceManager::GetDataSize() {
	unsigned int size = 0;
	for (auto it = m_list.begin(); it != m_list.end(); it++) {
		for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
			if (it2->second) {
				size += it2->second->GetDataSize();
			}
		}
	}
	return size;
}
