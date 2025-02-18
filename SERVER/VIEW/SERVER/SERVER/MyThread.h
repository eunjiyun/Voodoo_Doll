#include "main.h"
#include <sqlext.h>
#include <sql.h>
#include <sqltypes.h>

SOCKET g_s_socket, g_c_socket;
OVER_EXP g_a_over;
HANDLE h_iocp;

void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
void DB_Thread();
void process_packet(const int c_id, char* packet);
void worker_thread(HANDLE h_iocp);
void Timer_Thread();

/**
 * @brief SQL 에러 진단 정보를 처리하고 출력하는 함수
 * @param hHandle SQL 핸들
 * @param hType 핸들 타입
 * @param RetCode 반환 코드
 *
 * SQL 작업 수행 중 발생한 에러에 대한 상세 정보를 출력합니다.
 */
void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode) {
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS)
	{
		if (wcsncmp(wszState, L"01004", 5))
		{
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

/**
 * @brief 데이터베이스 연결 및 쿼리 처리를 담당하는 스레드 함수
 *
 * 주요 기능:
 * - ODBC를 통한 데이터베이스 연결 
 * - 회원가입(sign_up) 및 로그인(sign_in) 프로시저 처리
 * - 데이터베이스 이벤트 큐 모니터링 및 처리
 *
 * @note 데이터베이스 연결은 5초 타임아웃으로 설정됩니다.
 */
void DB_Thread()
{
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	SQLLEN OutSize{};

	// 한글 처리를 위한 로케일 설정
	setlocale(LC_ALL, "korean");

	SQLLEN cbID = 0, cbPWD = 0, cbClearTime = 0, cbCurStage = 0;

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);


			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				SQLWCHAR* connectionString = (SQLWCHAR*)L"DRIVER=SQL Server;SERVER=58.229.106.16;DATABASE=VooDooDoll_DB; UID=dbAdmin; PWD=2018180005;";

				retcode = SQLDriverConnect(hdbc, NULL, connectionString, SQL_NTS, NULL, 1024, NULL, SQL_DRIVER_NOPROMPT);

				// Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
					printf("ODBC Connect OK \n");
				}
				else
					HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);

				while (1)
				{
					DB_EVENT ev;
					if (db_queue.try_pop(ev)) {
						cout << "GET REQUEST\n";
						SQLWCHAR* param1 = ev.user_id;
						SQLWCHAR* param2 = ev.user_password;
						SQLINTEGER param3 = ev.session_id;
						auto requested_session = getClient(ev.session_id);
						if (requested_session == nullptr) {
							cout << "wrong session_id - DB Request\n";
							continue;
						}
						switch (ev._event) {
						case EV_SIGNUP: {
							retcode = SQLPrepare(hstmt, (SQLWCHAR*)L"{CALL sign_up(?, ?)}", SQL_NTS);
							if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
								SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 11, 0, (SQLPOINTER)param1, 0, NULL);
								SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 11, 0, (SQLPOINTER)param2, 0, NULL);

								retcode = SQLExecute(hstmt);
								if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
									SC_SIGN_PACKET p;
									p.success = true;
									p.type = SC_SIGNUP;
									p.size = sizeof(p);
									requested_session->do_send(&p);
									wcout << "SIGNUP SUCCEED \n";
								}
								else {
									SC_SIGN_PACKET p;
									p.success = false;
									p.type = SC_SIGNUP;
									p.size = sizeof(p);
									requested_session->do_send(&p);
									wcout << "SIGNUP FAILED \n";
									HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
								}
							}
							else {
								SC_SIGN_PACKET p;
								p.success = false;
								p.type = SC_SIGNUP;
								p.size = sizeof(p);
								requested_session->do_send(&p);
								printf("SQLPrepare failed\n");
								HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
							}
							SQLFreeStmt(hstmt, SQL_CLOSE);
							break;
						}
						case EV_SIGNIN: {
							wcscpy_s(requested_session->_name, sizeof(ev.user_id) / sizeof(ev.user_id[0]), ev.user_id);


							SC_SIGN_PACKET p;
							p.success = true;
							p.type = SC_SIGNIN;
							p.size = sizeof(p);
							requested_session->do_send(&p);
							wcout << "SIGNIN SUCCEED\n";

							retcode = SQLPrepare(hstmt, (SQLWCHAR*)L"{CALL sign_in(?, ?)}", SQL_NTS);
							if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
								SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 11, 0, (SQLPOINTER)param1, 0, NULL);
								SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 11, 0, (SQLPOINTER)param2, 0, NULL);

								retcode = SQLExecute(hstmt);
								if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
									SQLBindCol(hstmt, 4, SQL_C_LONG, &requested_session->cur_stage, sizeof(requested_session->cur_stage), &OutSize);
									retcode = SQLFetch(hstmt);
									if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
										SC_SIGN_PACKET p;
										p.success = true;
										p.type = SC_SIGNIN;
										p.size = sizeof(p);
										requested_session->do_send(&p);
										wcout << "SIGNIN SUCCEED\n";
									}
									else {
										SC_SIGN_PACKET p;
										p.success = false;
										p.type = SC_SIGNIN;
										p.size = sizeof(p);
										requested_session->do_send(&p);
										printf("SIGNIN FAILED - The ID does not exist.  \n");
									}
								}
								else {
									SC_SIGN_PACKET p;
									p.success = false;
									p.type = SC_SIGNIN;
									p.size = sizeof(p);
									requested_session->do_send(&p);
									printf("SIGNIN FAILED - The query has not been performed.  \n");
									HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
								}
							}
							else {
								SC_SIGN_PACKET p;
								p.success = false;
								p.type = SC_SIGNIN;
								p.size = sizeof(p);
								requested_session->do_send(&p);
								printf("SQLPrepare failed\n");
								HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
							}
							SQLFreeStmt(hstmt, SQL_CLOSE);
							break;
						}
						}
					}
					else this_thread::sleep_for(100ms);
				}

				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				}
				SQLDisconnect(hdbc);
			}
			SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
}


/**
 * @brief 타이머 이벤트를 처리하는 스레드 함수
 *
 * 타이머 큐를 모니터링하고 예약된 이벤트를 처리합니다.
 * 현재 구현된 이벤트:
 * - EV_MONSTER_UPDATE: 몬스터 상태 업데이트
 *
 * @note 이벤트는 지정된 wakeup_time에 처리됩니다.
 * @note 1ms 간격으로 타이머 큐를 확인합니다.
 */
void Timer_Thread()
{
	while (1)
	{
		TIMER_EVENT ev;
		auto current_time = high_resolution_clock::now();
		if (timer_queue.try_pop(ev)) {
			if (ev.wakeup_time > current_time) {
				timer_queue.push(ev);
				this_thread::sleep_for(1ms);
				continue;
			}
			switch (ev.event_id) {
			case EV_MONSTER_UPDATE: {
				OVER_EXP* ov = new OVER_EXP();
				ov->_comp_type = OP_NPC_UPDATE;
				PostQueuedCompletionStatus(h_iocp, 1, ev.room_id * 100 + ev.obj_id, &ov->_over);
			}
								  break;
			}
		}
		else this_thread::sleep_for(1ms);
	}
}

/**
 * @brief 클라이언트로부터 수신한 패킷을 처리하는 함수
 * @param c_id 클라이언트 ID
 * @param packet 수신된 패킷 데이터
 *
 * 처리하는 패킷 타입:
 * - CS_LOGIN: 게임 로그인 및 매칭
 * - CS_SIGNUP: 회원가입
 * - CS_SIGNIN: 로그인
 * - CS_HEARTBEAT: 위치 업데이트
 * - CS_ROTATE: 회전 정보
 * - CS_ATTACK: 공격 처리
 * - CS_INTERACTION: 상호작용(퍼즐 오브젝트 획득)
 * - CS_CHANGEWEAPON: 무기 변경
 */
void process_packet(const int c_id, char* packet)
{
	SESSION* CL = getClient(c_id);
	array<SESSION, MAX_USER_PER_ROOM>* Room = getRoom_Clients(c_id);
	if (CL == nullptr || Room == nullptr) {
		cout << "Wrong session_id - process_packet" << endl;
		return;
	}
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		CL->_state.store(ST_INGAME);

		// 3인 매칭 후 시작해야 하는 경우
		for (auto& pl : *Room) {
			if (pl._state.load() != ST_INGAME) return;
			pl.Initialize();
		}
		for (auto& pl : *Room) {
			pl.send_game_start_packet();
			for (auto& session : *Room) {
				if (pl._id == session._id) continue;
				pl.send_add_player_packet(&session);
			}
		}

		// 1인 게임 - 테스트용
		//CL.Initialize();
		//CL.send_game_start_packet();
		//for (auto& pl : Room) {
		//	if (pl._id == c_id || ST_INGAME != pl._state.load()) continue;
		//	pl.send_add_player_packet(&CL);
		//	CL.send_add_player_packet(&pl);
		//}
		//for (auto& monster : getRoom_Monsters(c_id)) {
		//	if (monster->alive.load() == false) continue;
		//	CL.send_summon_monster_packet(monster);
		//}
	}
				 break;
	case CS_SIGNUP: {
		CS_SIGN_PACKET* p = reinterpret_cast<CS_SIGN_PACKET*>(packet);
		DB_EVENT ev;
		ev._event = EV_SIGNUP;
		wcscpy_s(ev.user_id, sizeof(ev.user_id) / sizeof(wchar_t), p->id);
		wcscpy_s(ev.user_password, sizeof(ev.user_password) / sizeof(wchar_t), p->password);
		ev.session_id = c_id;
		db_queue.push(ev);
	}
				  break;
	case CS_SIGNIN: {
		CS_SIGN_PACKET* p = reinterpret_cast<CS_SIGN_PACKET*>(packet);
		DB_EVENT ev;
		ev._event = EV_SIGNIN;
		wcscpy_s(ev.user_id, sizeof(ev.user_id) / sizeof(wchar_t), p->id);
		wcscpy_s(ev.user_password, sizeof(ev.user_password) / sizeof(wchar_t), p->password);
		ev.session_id = c_id;
		db_queue.push(ev);
	}
				  break;
	case CS_HEARTBEAT: {
		CS_HEARTBEAT_PACKET* p = reinterpret_cast<CS_HEARTBEAT_PACKET*>(packet);
		CL->Update(p);
		for (auto& cl : *Room) {
			if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)  cl.send_update_packet(CL);
		}
	}
					 break;
	case CS_ROTATE: {
		CS_ROTATE_PACKET* p = reinterpret_cast<CS_ROTATE_PACKET*>(packet);
		CL->Rotate(p->cxDelta, p->cyDelta, p->czDelta);
		for (auto& cl : *Room) {
			if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)  cl.send_rotate_packet(CL);
		}
	}
				  break;
	case CS_ATTACK: {
		auto Monsters = getRoom_Monsters(CL->_id);
		if (Monsters == nullptr) {
			cout << "wrong session_id - processing attack" << endl;
			break;
		}
		CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
		for (auto& cl : *Room) {
			if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)   cl.send_attack_packet(CL);
		}
		XMFLOAT3 Cur_LookVector = CL->GetLookVector();
		XMFLOAT3 Cur_Pos = CL->GetPosition();
		switch (CL->weapon_type)
		{
		// BLADE: 근접 공격으로 거리 15 이내의 첫번째 몬스터에게 데미지
		case BLADE:
		{
			Cur_Pos = Vector3::Add(Cur_Pos, Vector3::ScalarProduct(Cur_LookVector, 15, false));
			for (auto& monster : *Monsters) {
				if (monster->alive == false) continue;
				lock_guard<mutex> mm{ monster->m_lock };
				if (monster->HP > 0 && Vector3::Length(Vector3::Subtract(Cur_Pos, monster->GetPosition())) < 30)
				{
					monster->HP -= 100;
					monster->target_id = CL->_id;
					if (monster->GetState() == NPC_State::Idle)
					{
						monster->SetState(NPC_State::Chase);
					}
					for (auto& cl : *Room) {
						if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)   cl.send_monster_damaged_packet(c_id, monster->m_id, monster->HP);
					}
					if (monster->HP <= 0) {
						monster->SetState(NPC_State::Dead);
					}
					break;
				}
			}
		}
		break;
		// GUN: 레이캐스트 방식으로 장애물에 막히는 원거리 공격
		case GUN:
		{
			XMVECTOR Bullet_Origin = XMLoadFloat3(&Cur_Pos);
			XMVECTOR Bullet_Direction = XMLoadFloat3(&Cur_LookVector);
			vector<Monster*> monstersInRange;

			for (auto& monster : *Monsters) {
				if (monster->alive == false) continue;
				lock_guard<mutex> monster_lock{ monster->m_lock };
				float bullet_monster_distance = Vector3::Length(Vector3::Subtract(monster->BB.Center, Cur_Pos));
				if (monster->HP > 0 && monster->BB.Intersects(Bullet_Origin, Bullet_Direction, bullet_monster_distance))
				{
					monstersInRange.push_back(monster);
				}
			}

			if (!monstersInRange.empty())
			{
				float minDistance = FLT_MAX;
				Monster* closestMonster = nullptr;
				for (auto& monster : monstersInRange)
				{
					lock_guard<mutex> monster_lock{ monster->m_lock };
					float distance = Vector3::Length(Vector3::Subtract(monster->BB.Center, Cur_Pos));
					if (distance < minDistance)
					{
						minDistance = distance;
						closestMonster = monster;
					}
				}
				if (closestMonster)
				{
					lock_guard<mutex> monster_lock{ closestMonster->m_lock };
					int _min = static_cast<int>(min(Cur_Pos.z / AREA_SIZE, closestMonster->BB.Center.z / AREA_SIZE));
					int _max = static_cast<int>(max(Cur_Pos.z / AREA_SIZE, closestMonster->BB.Center.z / AREA_SIZE));
					for (int i = _min; i <= _max; i++)
					{
						for (auto& obj : Obstacles[i])
						{
							float bullet_obstacle_distance = Vector3::Length(Vector3::Subtract(obj->m_xmOOBB.Center, Cur_Pos));
							if (obj->m_xmOOBB.Intersects(Bullet_Origin, Bullet_Direction, bullet_obstacle_distance) &&
								bullet_obstacle_distance < minDistance) {
								return;
							}
						}
					}
					closestMonster->HP -= 100;
					closestMonster->target_id = CL->_id;
					if (closestMonster->GetState() == NPC_State::Idle)
					{
						closestMonster->SetState(NPC_State::Chase);
					}
					for (auto& cl : *Room) {
						if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)   cl.send_monster_damaged_packet(c_id, closestMonster->m_id, closestMonster->HP);
					}
					if (closestMonster->HP <= 0) {
						closestMonster->SetState(NPC_State::Dead);
					}
					return;
				}
			}
			break;
		}
		// PUNCH: 근접 공격으로 거리 10 이내의 첫번째 몬스터에게 데미지
		case PUNCH:
		{
			Cur_Pos = Vector3::Add(Cur_Pos, Vector3::ScalarProduct(Cur_LookVector, 10, false));
			for (auto& monster : *Monsters) {
				if (monster->alive == false) continue;
				lock_guard<mutex> mm{ monster->m_lock };
				if (monster->HP > 0 && Vector3::Length(Vector3::Subtract(Cur_Pos, monster->GetPosition())) < 20)
				{
					monster->HP -= 50;
					monster->target_id = CL->_id;
					if (monster->GetState() == NPC_State::Idle)
					{
						monster->SetState(NPC_State::Chase);
					}
					for (auto& cl : *Room) {
						if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)   cl.send_monster_damaged_packet(c_id, monster->m_id, monster->HP);
					}
					if (monster->HP <= 0) {
						monster->SetState(NPC_State::Dead);
					}
					break;
				}
			}
			break;
		}
		}
	}
				  break;
	case CS_INTERACTION: {
		CS_INTERACTION_PACKET* p = reinterpret_cast<CS_INTERACTION_PACKET*>(packet);
		int cur_stage = CL->cur_stage.load();
		for (int i = 0; i < Key_Items[cur_stage].size(); i++) {
			if (Key_Items[cur_stage][i].m_xmOOBB.Intersects(CL->m_xmOOBB)) {
				for (auto& cl : *Room) {
					cl.clear_percentage += Key_Items[cur_stage][i].percent;
					if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)   cl.send_interaction_packet(cur_stage, i);
				}
			}
		}

		if (CL->clear_percentage >= 1.f && get_remain_Monsters(CL->_id) == 0) {
			for (auto& cl : *Room) {
				if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)
					cl.send_open_door_packet(CL->cur_stage);
				cl.clear_percentage = 0.f;
				if (cl.cur_stage >= 0) cl.clear_percentage = 1.f;
			}
		}
	}
					   break;
	case CS_CHANGEWEAPON: {
		CS_CHANGEWEAPON_PACKET* p = reinterpret_cast<CS_CHANGEWEAPON_PACKET*>(packet);
		CL->weapon_type = static_cast<WEAPON_TYPE>((CL->weapon_type + 1) % 3);
		for (auto& cl : *Room) {
			if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)   cl.send_changeweapon_packet(CL);
		}
	}
						break;
	}
}

/**
 * @brief IOCP 워커 스레드 함수
 * @param h_iocp IOCP 핸들
 *
 * 처리하는 작업 타입:
 * - OP_ACCEPT: 새로운 클라이언트 연결 수락
 * - OP_RECV: 데이터 수신 처리
 * - OP_SEND: 데이터 송신 완료 처리
 * - OP_NPC_UPDATE: NPC 상태 업데이트
 *
 */
void worker_thread(HANDLE h_iocp)
{
	while (1) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) {
				cout << "Accept Error" << endl;
			}
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND)
					delete ex_over;
			}
			continue;
		}

		if ((0 == num_bytes) && ((ex_over->_comp_type == OP_RECV) || (ex_over->_comp_type == OP_SEND))) {
			disconnect(static_cast<int>(key));
			if (ex_over->_comp_type == OP_SEND)
				delete ex_over;
			continue;
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();
			if (client_id != -1) {
				SESSION* CL = getClient(client_id);
				if (CL == nullptr) {
					cout << "wrong session_id - Accept Session" << endl;
					break;
				}
				CL->_state.store(ST_ALLOC);
				CL->_socket = g_c_socket;
				CL->_id = client_id;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket),
					h_iocp, client_id, 0);
				CL->do_recv();
			}
			else {
				cout << "Max user exceeded.\n";
				closesocket(g_c_socket);
			}
			g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
		}
					  break;
		case OP_RECV: {
			SESSION* CL = getClient(static_cast<int>(key));
			if (CL == nullptr) {
				cout << "wrong session_id - Recv Packet" << endl;
				break;
			}
			if (CL->_state.load() == ST_DEAD) {
				break;
			}
			int remain_data = num_bytes + CL->_prev_remain;
			char* p = ex_over->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(key), p);
					p += packet_size;
					remain_data -= packet_size;
				}
				else break;
			}
			CL->_prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			CL->do_recv();
		}
					break;
		case OP_SEND: {
			delete ex_over;
		}
					break;
		case OP_NPC_UPDATE: {
			int roomNum = static_cast<int>(key) / 100;
			short mon_id = static_cast<int>(key) % 100;
			auto& monster = monsters[roomNum][mon_id];

			if (monster->alive.load() == true) {
				{
					// 몬스터의 상태를 주기적으로 업데이트하고 모든 클라이언트에게 동기화
					lock_guard<mutex> mm{ monster->m_lock };
					monster->Update(duration_cast<milliseconds>(high_resolution_clock::now() - monster->recent_updateTime).count() / 1000.f);
					monster->recent_updateTime = high_resolution_clock::now();
				}
				for (auto& cl : clients[roomNum]) {
					if (cl._state.load() == ST_INGAME || cl._state.load() == ST_DEAD)  cl.send_monster_update_packet(monster);
				}

				// 업데이트 후 다음 업데이트를 위한 타이머 이벤트 등록
				TIMER_EVENT ev{ roomNum, mon_id, monster->recent_updateTime + 100ms, EV_MONSTER_UPDATE };
				timer_queue.push(ev);
			}
			delete ex_over;
		}
						  break;
		}
	}
}
