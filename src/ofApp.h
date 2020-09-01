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
	pair<int, int> pos; // ������ġ
	time_t startTime; // �÷��̾� ���� ���۽ð�
	int starscore; // �� � �Ծ�����
	int timescore; // time bonus ����
	int totalscore; // �̷� �� ã���� time bonus + �߰��� ȹ���� star
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
		void drawFirstpage(); // ���� �ʱ�ȭ�� �׸�
		void drawStar(int x, int y); // �Ѻ��� ���̰� �־����� �� �׸�
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
		void mazeGenerate(); // �̷� ���� �Լ�
		bool check_move(int x, int y, int d); // �̷� ���� �����Լ�1
		pair <int, int> process_d(int x, int y, int d); // �̷� ���� �����Լ�2
		void create_graph(); // �̷� ��� �����ϴ� �׷��� ����
		void add_edge(int x, int y, int d); // �׷����� �̷� ��� ����
		void mazedraw(); // �̷θ� �׷��ִ� �Լ�
		bool findPath(); // �̷� ��� ã���ִ� �Լ�
		void pathdraw(); // �̷� ��� ���� �����ִ� �Լ�
		void playgame(int key); // ���� ���� �Լ�
		bool play_check(int x1, int y1, int x2, int y2); // ���� ����ÿ� �� �� �ִ��� �˻��ϴ� �Լ�
		void drawcharacter(); // �÷��̾� �����ܰ� ���� ��� �׷��ִ� �Լ�
		void calc_score(); //isgoal�϶� ���� ������ִ� �Լ�
		void insert_bst(char* n, int s); // �̸��� ������ �����ϸ� �˸��� ��ġ�� ��ŷ ����
		void make_rankfile(); // ���α׷� ������ ���� ��ũ���Ͽ� ��ŷ ����
		void drawranking();// ��ŷ�޴� ���ý� ��ŷ ������

		int HEIGHT;//�̷��� ����
		int WIDTH;//�̷��� �ʺ�
		int WALL; // �̷��� �� ���� ����
		int STARS; //�� ����
		int TIME; // Ÿ�� ���ʽ� �ִ� ���ѽð�
		player USER; // ���� ���� �÷��̾�
		ofFile rankfile; // ��ŷ�����ϴ� ����

		bstnode* ranktree; // ��ŷ�� ��Ÿ���� binary search tree
		vector<node*> path_g; // ��θ� �׷����� ��Ÿ����.
		int target_x, target_y; // �̷� ������
		vector<pair<int,int>> maze_path; // �̷� ��� ����
		vector<pair<int, int>> stars_pos; // ���� ��ġ ����

		int isMaze; //���� ����(������ ���� �����) �̷ΰ� �ִ��� �Ǵ��ϴ� ����. 0�̸� �ȿ��Ȱ� 1�̸� ���ȴ�.
		int isGame; // ������ ���������� �Ǵ��ϴ� ����
		int isFirstpage; // ó���������� �������� �Ǵ��ϴ� ����
		int isPath;//Show Answer�� ������״��� �Ǵ��ϴ� ����. 0�̸� ������߰� 1�̸� �����ߴ�.
		int isRanking; // ��ŷ����� ���õǾ����� �Ǵ��ϴ� ����
		int isExit; // ���� ���ø޴��� exit���� �Ǵ��ϴ� ���� -> freememory���� bst�� �޸� ������ ���δ�.

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
