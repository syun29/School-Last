#include "Base.h"
std::list<Base*> Base::m_list;
Base::Base(unsigned int type)
	:m_kill(0), m_type(type), m_pos(0, 0, 0), m_rot(0, 0, 0) {
}
Base::~Base() {

}
void Base::SetKill() {
	//削除フラグON
	if (!m_kill) m_kill = 2;
}
void Base::Update() {

}
void Base::Render() {

}
void Base::Draw() {

}
void Base::Collision(Base* b) {

}
void Base::KillALL() {
	//全ての削除フラグをONにする
	for (auto& b : m_list) {
		b->SetKill();
	}
}
void Base::Kill(unsigned int mask)
{
	for (auto& b : m_list) {
		if ((1 << (int)b->m_type) & (mask))
			b->SetKill();
	}
}
void Base::ClearInstance()
{

	auto itr = m_list.begin();
	//末尾まで繰り返す
	while (itr != m_list.end()) {
		//削除
		delete* itr;
		//リストから除外する
		//次のオブジェクトを受け取る
		itr = m_list.erase(itr);
	}
}
void Base::CheckKillALL() {
	auto itr = m_list.begin();
	//末尾まで繰り返す
	while (itr != m_list.end()) {
		//削除チェック
		if ((*itr)->m_kill) {
			if (--(*itr)->m_kill == 0) {
				//削除
				delete* itr;
				//リストから除外する
				//次のオブジェクトを受け取る
				itr = m_list.erase(itr);
				continue;
			}
		}
		//次のオブジェクト
		itr++;
	}
}
void Base::UpdateALL() {
	for (auto& b : m_list) {
		if(!b->m_kill) b->Update();
	}
}
void Base::RenderALL() {
	for (auto& b : m_list) {
		if (!b->m_kill) b->Render();
	}
}
void Base::DrawALL() {
	for (auto& b : m_list) {
		if (!b->m_kill) b->Draw();
	}
}

void Base::CollisionALL() {
	auto itr = m_list.begin();
	//末尾まで繰り返す
	while (itr != m_list.end()) {
		if (!(*itr)->m_kill) {
			auto ct = std::next(itr);
			while (ct != m_list.end()) {
				if (!(*ct)->m_kill) {
					(*itr)->Collision(*ct);
					(*ct)->Collision(*itr);
				}
				//次のオブジェクト
				ct++;
			}
		}
		//次のオブジェクト
		itr++;
	}

}

void Base::Add(Base* b) {
	//
	auto itr = m_list.begin();
	//末尾まで繰り返す
	while (itr != m_list.end()) {
		if ((*itr)->GetType() > b->GetType()) {
			m_list.insert(itr, b);
			return;
		}
		//次のオブジェクト
		itr++;
	}
	m_list.push_back(b);

}



Base* Base::FindObject(unsigned int type) {

	for (auto& b : m_list) {
		if (b->GetType() == type && !b->m_kill) return b;
	}
	return nullptr;
}

std::vector<Base*> Base::FindObjects(unsigned int type)
{
	std::vector<Base*> list;

	for (auto& b : m_list) {
		if (b->GetType() == type && !b->m_kill) list.push_back(b);
	}
	return list;
}



