/*

	ofxWinMenu basic example - ofApp.h

	Copyright (C) 2016-2017 Lynn Jarvis.

	http://www.spout.zeal.co

	=========================================================================
	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
	=========================================================================
*/

#pragma once

#include "ofMain.h"
#include "ofxWinMenu.h" // Menu addon
#include <utility>
#include <time.h>

typedef struct node_{
	int x, y;
	node_* link;	
}node;

typedef struct {
	pair<int, int> pos; // 현재위치
	time_t startTime; // 플레이어 게임 시작시간
	int starscore; // 별 몇개 먹었는지
	int timescore; // time bonus 점수
	int totalscore; // 미로 다 찾으면 time bonus + 중간에 획득한 star
	string name;
}player;

typedef struct bstnode_ {
	char name[21];
	int score;
	struct bstnode_* lchild;
	struct bstnode_* rchild;
}bstnode;

class ofApp : public ofBaseApp {

	public:

		void setup();
		void update();
		void draw();
		void drawFirstpage(); // 게임 초기화면 그림
		void drawStar(int x, int y); // 한변의 길이가 주어지면 별 그림
		void keyPressed(int key); // Traps escape key if exit disabled
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		bool readFile();

		void freeMemory();
		void mazeGenerate(); // 미로 생성 함수
		bool check_move(int x, int y, int d); // 미로 생성 내부함수1
		pair <int, int> process_d(int x, int y, int d); // 미로 생성 내부함수2
		void create_graph(); // 미로 경로 저장하는 그래프 생성
		void add_edge(int x, int y, int d); // 그래프에 미로 경로 저장
		void mazedraw(); // 미로를 그려주는 함수
		bool findPath(); // 미로 경로 찾아주는 함수
		void pathdraw(); // 미로 경로 답을 보여주는 함수
		void playgame(int key); // 게임 실행 함수
		bool play_check(int x1, int y1, int x2, int y2); // 게임 실행시에 갈 수 있는지 검사하는 함수
		void drawcharacter(); // 플레이어 아이콘과 이전 경로 그려주는 함수
		void calc_score(); //isgoal일때 점수 계산해주는 함수
		void insert_bst(char* n, int s); // 이름과 점수를 전달하면 알맞은 위치에 랭킹 삽입
		void make_rankfile(); // 프로그램 끝나기 전에 랭크파일에 랭킹 저장
		void drawranking();// 랭킹메뉴 선택시 랭킹 보여줌

		int HEIGHT;//미로의 높이
		int WIDTH;//미로의 너비
		int WALL; // 미로의 한 벽의 길이
		int STARS; //별 개수
		int TIME; // 타임 보너스 있는 제한시간
		player USER; // 현재 게임 플레이어
		ofFile rankfile; // 랭킹저장하는 파일

		bstnode* ranktree; // 랭킹을 나타내는 binary search tree
		vector<node*> path_g; // 경로를 그래프로 나타낸다.
		int target_x, target_y; // 미로 도착점
		vector<pair<int,int>> maze_path; // 미로 경로 저장
		vector<pair<int, int>> stars_pos; // 별들 위치 저장

		int isMaze; //현재 열린(게임을 위해 설계된) 미로가 있는지 판단하는 변수. 0이면 안열렸고 1이면 열렸다.
		int isGame; // 게임이 실행중인지 판단하는 변수
		int isFirstpage; // 처음페이지를 보여줄지 판단하는 변수
		int isPath;//Show Answer를 실행시켰는지 판단하는 변수. 0이면 실행안했고 1이면 실행했다.
		int isRanking; // 랭킹기능이 선택되었는지 판단하는 변수
		int isExit; // 현재 선택메뉴가 exit인지 판단하는 변수 -> freememory에서 bst의 메모리 해제에 쓰인다.

		// Menu
		ofxWinMenu * menu; // Menu object
		void appMenuFunction(string title, bool bChecked); // Menu return function

		// Used by example app
		ofTrueTypeFont myFont;
        ofImage myImage;
		float windowWidth, windowHeight;
		HWND hWnd; // Application window
		HWND hWndForeground; // current foreground window

		// Example menu variables
		bool bFullscreen;
		bool bTopmost;
		// Example functions
 		void doFullScreen(bool bFull);
		void doTopmost(bool bTop);

};
