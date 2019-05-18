#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define LEFT 104
#define UP 107
#define RIGHT 108
#define DOWN 106
#define UNDO 117
#define NEW  110
#define REPLAY 114
#define EXIT 101
#define SAVE 115
#define FILE_LOAD 102
#define DISPLAY_HELP 100
#define TOP 116
#define LF 10
#define CR 13

#define WALL 35
#define EMPTY 46
#define GOLD 36
#define PLAYER 64
#define STORAGE 79

#define NUMBER_OF_MAPS 5

#define MAPSIZE 31


// 위치 정보를 가지는 구조체
typedef struct _tagPosition {
	int x;						
	int	y;
}POSITION, *PPOSITION;

// 맵의 정보를 가지고 있는 구조체
typedef struct _tagMapData {
	int map[MAPSIZE][MAPSIZE];	// 맵 정보를 저장하는 배열
	int width, height;			// 맵의 높이와 길이
	POSITION playerInitPos;		// 플레이어의 시작점

}MAPDATA, *PMAPDATA;

// 움직인 정보를 가지는 구조체
typedef struct _tagMoveInfo {
	POSITION delta;				// 얼마만큼 움직였는지
	POSITION goldPosition;		// 움직인 후 금의 위치
}MOVEINFO, *PMOVEINFO;

// 랭킹 정보를 가지는 구조체
typedef struct _tagRankingInfo {
	char name[10];				// 플레이어의 이름
	int moveCount;				// 움직인 횟수
}RANKING, *PRANKING;

int getch();				// 화면에 문자를 출력하지 않고 입력을 받는 함수
void gotoxy(int x, int y);		// 화면의 커서를 움직이는 함수
int Len(char *s);				// 문자열의 길이를 출력하는 함수
int MapLoading();				// map파일로부터 맵을 로딩하는 함수
int SetMap(int level);			// 현재 플레이할 맵을 레벨이 level인 맵으로 변경
int IsInMap(POSITION _pos);		// _pos가 맵 안에 있는 위치인지
void Input();					// 입력을 담당하는 함수
void Render();					// 화면 출력을 담당하는 함수
void Move(int delX,int delY,int undoMoving); // x축으로 delX만큼, y로 delY 만큼
void Undo();					// Undo 기능
void New();						// 새로시작
void DisplayHelp();				// 명령어를 출력하는 함수
int Save();						// 현재 맵 상태를 저장하는 함수
int FileLoad();					// sokoban파일로부터 저장된 내용을 불러와 적용시키는 함수
int RankingSave();				// 랭킹을 저장하는 함수
int RankingLoad();				// top파일로부터 랭킹 정보를 가져오는 함수 
void RankingDisplay();			// 랭킹을 출력하는 함수
int Clear();					// 게임을 클리어 했는지 확인하는 함수

// 저장되어야할 정보
char		name[10];			// 이름
int			movingCount;		// 움직인 개수
int			undoCount = 5;		// undo할 수 있는 횟수
int			currentLevel = 0;	// 현재 맵의 레벨 0 ~ 4
MAPDATA		cMapData;			// 여기의 map 변수에는 금의 위치만 표시
POSITION	cPos;				// 캐릭터의 현재 위치
MOVEINFO	moveInfo[5];		// 최근 5번의 이동을 담은 변수

// 저장되지 않아도 되는 정보
MAPDATA		mapData[NUMBER_OF_MAPS];		// File로 부터 받아온 맵 정보
int			topPressedBeforeFrame;			// 이전 프레임에서 TOP키가 눌렸는지
int			showTopLevel = 0;				// 랭킹을 볼 맵의 레벨, 0 = 전체, 1 ~ 5 = 레벨
int			isPlay = 1;						// 현재 게임이 실행중인지.
RANKING		rankingList[NUMBER_OF_MAPS][5];	// [레벨][5위까지][이름, 이동 수]


/*
- h(왼쪽), j(아래), k(위), l(오른쪽) : 왼쪽 아래 위 오른쪽 창고지기 조정
- u(undo) : 5 최대 번 할 수 있음
- r(replay) : ( ) 현재 맵을 처음부터 다시 시작 움직임 횟수는 계속 유지
- n(new) : ( ) 첫 번째 맵부터 다시 시작 움직임 횟수 기록 삭제
- e(exit) : . 게임 종료 종료하기 전 필요한 정보 저장해야 함
- s(save) : . sokoban 현재 상태 파일에 저장 파일 이름은 으로 하고 다음에 다시 게임을 연속해서 할
수 있도록 모든 상태 저장해야 함
- f(file load) : sokoban save 파일에서 저장된 내용을 읽어 시점에서부터 이어서 게임하게 함
- d(display help) : 명령 내용 보여줌
- t(top) : . t . t 게임 순위 보여줌 만 입력하면 전체 순위 다음 숫자가 오면 해당 맵W의 순위
*/

int main() {

	// map 파일로 부터 맵 정보를 읽어옴
	if (!MapLoading()) {
		fprintf(stderr,"MapLoading() Error\n");
		return 0;
	}

	printf("이름을 입력하십시오: ");
	scanf("%s", name);

	// 처음 맵의 정보를 가져와서 적용함
	if (!SetMap(0)) {
		fprintf(stderr, "Init SetMap() Error\n");
		return 0;
	}
	RankingLoad(); // 랭킹 정보 로드
	
	while (isPlay) {
		Input(); // 입력 처리 관련 함수
		Render(); // 화면에 맵 출력

		if (Clear()) { // 게임 클리어 확인

			system("clear");
			RankingSave(); // 랭킹을 갱신 후 저장
			printf("#####################################\n");
			printf("#                                   #\n");
			printf("#                                   #\n");
			printf("#                                   #\n");
			printf("#              %02d Clear             #\n",currentLevel + 1);
			printf("#                                   #\n");
			printf("#                                   #\n");
			printf("#                                   #\n");
			printf("#####################################\n");

			getch();



			currentLevel++;
			New(); // 다음 레벨의 새로운 맵을 불러옴
		}
	}
	
}

void Input() {

	int topPressed = 0;
	char c = getch();
	switch (c) {
	case LEFT:
		Move(-1, 0,0);
		break;
	case UP:
		Move(0, -1,0);
		break;
	case RIGHT:
		Move(1, 0,0);
		break;
	case DOWN:
		Move(0, 1,0);
		break;
	case UNDO: 
		Undo();
		break;
	case NEW:
		New();
		break;
	case REPLAY :
		SetMap(currentLevel);
		break;
	case SAVE:
		Save();
		break;
	case FILE_LOAD:
		FileLoad();
		break;
	case DISPLAY_HELP:
		DisplayHelp();
		break;
	case TOP:
		topPressed = 1;
		break;
	case EXIT:
		Save();
		isPlay = 0;
		break;
	case LF: // Lind Feed 혹은
	case CR: // Carriage return 이면, 즉 엔터가 눌렸으면
		if (topPressedBeforeFrame)
		{
			RankingDisplay();
			showTopLevel = 0;
		}
	default:
		if (topPressedBeforeFrame) { // 전의 입력이 TOP이였는지 확인
			if (c - '0' >= 1 && c - '0' <= NUMBER_OF_MAPS) { // 입력 받은 키가 '1' ~ '5'인지 확인
				topPressed = 1;
				showTopLevel = c - '0';
			}
		}
		else showTopLevel = 0;

	}

	topPressedBeforeFrame = topPressed;
}

// 화면에 맵을 출력하는 함수
void Render() {
	gotoxy(0, 0);

	for (int y = 0; y < mapData[currentLevel].height; y++, printf("\n")) {
		for (int x = 0; x < mapData[currentLevel].width; x++) {
			if (cPos.x == x && cPos.y == y)
				printf("%c", PLAYER);
			else if (cMapData.map[y][x] != EMPTY)
				printf("%c", GOLD);
			else if (mapData[currentLevel].map[y][x] == WALL || mapData[currentLevel].map[y][x] == STORAGE)
				printf("%c",mapData[currentLevel].map[y][x]);
			else
				printf(" ");
		}
	}

	printf("\n");
	printf("MovingCount: %5d\n",movingCount);
	printf("undoCount  : %5d\n", undoCount);
}

void Move(int delX, int delY, int undoMoving) {
	POSITION goldPos = { -1,-1 };
	POSITION _pos = { cPos.x + delX, cPos.y + delY };
	if (!IsInMap(_pos))
		return;

	if (mapData[currentLevel].map[_pos.y][_pos.x] == WALL)
		return;

	if (cMapData.map[_pos.y][_pos.x] == GOLD)
	{
		goldPos.x = _pos.x + delX;
		goldPos.y = _pos.y + delY;
		if (!IsInMap(goldPos))
			return;
		if (mapData[currentLevel].map[goldPos.y][goldPos.x] == WALL)
			return;
		if (cMapData.map[goldPos.y][goldPos.x] != EMPTY)
			return;

		cMapData.map[goldPos.y][goldPos.x] = GOLD;
		cMapData.map[_pos.y][_pos.x] = EMPTY;

	}
	cPos = _pos;
	movingCount++;

	if (!undoMoving) {
		MOVEINFO pInfo = { delX * -1,delY * -1,goldPos };

		for (int i = 4; i > 0; i--) {
			moveInfo[i] = moveInfo[i - 1];
		}
		moveInfo[0] = pInfo;
	}
}

void Undo() {
	if (undoCount <= 0)
		return;
	if (moveInfo[0].delta.x == 0 && moveInfo[0].delta.y == 0)
		return;


	Move(moveInfo[0].delta.x, moveInfo[0].delta.y,1);
	if (moveInfo[0].goldPosition.x != -1 && moveInfo[0].goldPosition.y != -1)	{
		POSITION goldPos = { moveInfo[0].goldPosition.x + moveInfo[0].delta.x,moveInfo[0].goldPosition.y + moveInfo[0].delta.y };

		cMapData.map[moveInfo[0].goldPosition.y][moveInfo[0].goldPosition.x] = EMPTY;
		cMapData.map[goldPos.y][goldPos.x] = GOLD;

	}
	undoCount--;
	movingCount++;

	for (int i = 0; i < 4; i++) {
		moveInfo[i] = moveInfo[i + 1];
	}
}

void New() {
	movingCount = 0;
	undoCount = 5;
	SetMap(currentLevel);
}

void DisplayHelp() {
	system("clear");
	printf("h(왼쪽), j(아래), k(위), l(오른쪽)\n");
	printf("u(undo)\n");
	printf("r(replay)\n");
	printf("n(new)\n");
	printf("e(exit)\n");
	printf("s(save)\n");
	printf("f(file load)\n");
	printf("d(display help)\n");
	printf("t(top)\n");
	getch();
	system("clear");
}

int Save() {
	FILE *sokoban;

	if((sokoban = fopen("sokoban.txt","w") ) == NULL) {
		fprintf(stderr,"sokoban.txt 파일을 불러오지 못 했습니다.\n");
		return 0;
	}

	fprintf(sokoban, "%s\n", name);
	fprintf(sokoban, "%d\n", movingCount);
	fprintf(sokoban, "%d\n", undoCount);
	fprintf(sokoban, "%d\n", currentLevel);
	// 맵 정보 저장
	for (int y = 0; y < cMapData.height; y++)
		for (int x = 0; x < cMapData.width; x++)
			fprintf(sokoban, "%d ", cMapData.map[y][x]);

	fprintf(sokoban, "%d %d\n", cPos.x, cPos.y);

	for (int i = 0; i < 5; i++) {
		fprintf(sokoban, "%d %d %d %d ", moveInfo[i].delta.x, moveInfo[i].delta.y, moveInfo[i].goldPosition.x, moveInfo[i].goldPosition.y);
	}

	fclose(sokoban);
	return 1;
}

int FileLoad() {
	FILE *sokoban;

	if (access("sokoban.txt", 0) == -1) {
		fprintf(stderr, "sokoban.txt 파일이 존재하지 않습니다.\n");
		return 0;
	}

	if ((sokoban = fopen("sokoban.txt", "r")) == NULL) {
		fprintf(stderr,"sokoban.txt 파일을 불러오지 못 했습니다.\n");
		return 0;
	}
	int level;

	fscanf(sokoban, "%s", &name);
	fscanf(sokoban, "%d", &movingCount);
	fscanf(sokoban, "%d", &undoCount);
	fscanf(sokoban, "%d", &currentLevel);
	
	cMapData.height = mapData[currentLevel].height;
	cMapData.width = mapData[currentLevel].width;
	for (int y = 0; y < mapData[currentLevel].height; y++)
		for (int x = 0; x < mapData[currentLevel].width; x++)
			fscanf(sokoban, "%d", &cMapData.map[y][x]);
	fscanf(sokoban, "%d %d", &cPos.x, &cPos.y);
	for (int i = 0; i < 5; i++) {
		fscanf(sokoban, "%d %d %d %d", &moveInfo[i].delta.x, &moveInfo[i].delta.y, &moveInfo[i].goldPosition.x, &moveInfo[i].goldPosition.y);
	}
	fclose(sokoban);

	system("clear");
	Render();
	return 1;
}

int RankingSave() {
	FILE *ranking;


	for (int i = 0; i < NUMBER_OF_MAPS; i++)
	{
		if (rankingList[currentLevel][i].moveCount == 0)
		{
			strcpy(rankingList[currentLevel][i].name, name);
			rankingList[currentLevel][i].moveCount = movingCount;
			break;
		}

		if (rankingList[currentLevel][i].moveCount > movingCount)
		{
			for (int j = i; j < NUMBER_OF_MAPS - 1; j++)
			{
				strcpy(rankingList[currentLevel][i + 1].name, rankingList[currentLevel][i].name);
				rankingList[currentLevel][i + 1].moveCount = rankingList[currentLevel][i].moveCount;
			}
			strcpy(rankingList[currentLevel][i].name, name);
			rankingList[currentLevel][i].moveCount = movingCount;
			break;
		}
	}

	if ((ranking = fopen("ranking.txt", "w")) == NULL)
	{
		fprintf(stderr, "ranking.txt 파일을 불러오지 못 했습니다.\n");
		return 0;
	}

	for (int i = 0; i < NUMBER_OF_MAPS; i++) {
		fprintf(ranking, "RankingLevel%d\n", i);
		for (int j = 0; j < 5; j++) {
			if (rankingList[i][j].moveCount > 0) {
				fprintf(ranking, "%s %d\n", rankingList[i][j].name, rankingList[i][j].moveCount);
			}
			else break;
		}
	}
	fclose(ranking);


	return 1;
}

int RankingLoad() {
	FILE *ranking;

	int level = 0;

	if (access("ranking.txt", 0) == -1)
	{
		return 0;
	}

	if ((ranking = fopen("ranking.txt", "r")) == NULL)
	{
		fprintf(stderr, "ranking.txt 파일을 불러오지 못 했습니다.\n");
		return 0;
	}

	char s[10000];
	int rankingNum = 0;
	int strlen = 0;

	while ((fscanf(ranking, "%s", s)) != EOF)
	{
		strlen = Len(s);

		if (strlen == (Len("RankingLevel") +1)&&strncmp("RankingLevel",s,strlen-1) == 0) {
			level = s[strlen-1] - '0';
			rankingNum = 0;
		}
		else {
			strncpy(rankingList[level][rankingNum].name, s,strlen);
			fscanf(ranking, "%d", &rankingList[level][rankingNum].moveCount);
			rankingNum++;
		}
	}
	fclose(ranking);
	return 1;
}

void RankingDisplay() {
	
	system("clear");

	if (showTopLevel == 0) {
		for (int i = 0; i < NUMBER_OF_MAPS; i++) {
			printf("map %d\n", i + 1);
			for (int j = 0; j < 5; j++) {
				if (rankingList[i][j].moveCount > 0) {
					printf("%s %d\n", rankingList[i][j].name, rankingList[i][j].moveCount);
				}
				else{
				    	if(j == 0) 
					{
					    printf("None\n");
					    break;
					}
				}
			}
		}
	}
	else {
		printf("map %d\n", showTopLevel);
		for (int i = 0; i < 5; i++) {
			if (rankingList[showTopLevel - 1][i].moveCount > 0) {
				printf("%s %d\n", rankingList[showTopLevel - 1][i].name, rankingList[showTopLevel - 1][i].moveCount);
			}
			else{
			    if(i ==0)
			    {
				printf("None\n");
				break;
			    }
			}
		}
	}

	getch();
	system("clear");
}

int MapLoading() {
	FILE *mapFile;

	if (access("map.txt", 0) == -1) {
		fprintf(stderr, "map.txt 파일이 존재하지 않습니다.\n");
		return 0;
	}
	
	if ((mapFile = fopen("map.txt", "r")) == 0) {
		printf("map.txt 파일을 불러오지 못 했습니다.\n");
		return 0;
	}

	char s[MAPSIZE];	// 한 줄씩 받아오기 위한 변수
	int returnValue = 1;	// 리턴 값 1 == 정상작동, 0 == 비정상작동

	int strLength = 0;	// 현재 읽어드린 문자열의 길이 
	int level = -1;		// 현재 로드하는 맵의 레벨
	int currentY = 0;	// 현재 로드 중인 맵의 Y축 위치

	int goldCount = 0, storageCount = 0; // 금괴 개수와 저장소 개수가 맞는 확인하는 변수
	while (fscanf(mapFile, "%s",s) != EOF) { // 입력이 존재하면
		strLength = Len(s);
		if (strLength > 30)		{
			printf("적합하지 않은 map파일입니다.\n");
			returnValue = 0;
			break;
		}

		if (strLength == 1) {
			if (s[0] == 'e')
				break;
			else if (s[0] - '0' > 0) {
				if (goldCount != storageCount){
					printf("%d번 맵의 골드와 보관장소의 개수가 맞지 않습니다.\n", level );
					returnValue = 0;
				}
				goldCount = 0;
				storageCount = 0;

				level = s[0] - '0';
				currentY = 0;
			}
		}
		else {
			if (level -1 >=0) {
				if (currentY == 0)
					mapData[level - 1].width = strLength;
				for (int i = 0; i < strLength; i++) {
					if (s[i] == GOLD)
						goldCount++;
					else if (s[i] == STORAGE)
						storageCount++;

					if (s[i] == PLAYER) {
						mapData[level - 1].playerInitPos.x = i;
						mapData[level - 1].playerInitPos.y = currentY;
						mapData[level - 1].map[currentY][i] = EMPTY;
					}
					else
						mapData[level - 1].map[currentY][i] = s[i];
				}
				mapData[level - 1].height = ++currentY;
				if (currentY + 1 > 30) {
					printf("적합하지 않은 map파일입니다.\n");
					returnValue = 0;
					break;
				}
			}
		}
		//printf("%s\n", s);
	}

	fclose(mapFile);
	return returnValue;
}


int SetMap(int level) {
    	
	currentLevel = level; // 현재 게임 레벨을 level로 설정
	if (!(mapData[currentLevel].width > 0 && mapData[currentLevel].height > 0)) // level에 해당하는 맵이 존재하지 않으면 게임 종료
		return 0;

	cMapData.width = mapData[currentLevel].width; // 현재 맵의 금괴 정보를 담는 cMapData의 width, height 설정
	cMapData.height = mapData[currentLevel].height;

	// cMapData는 금괴의 정보를 담고 있는 배열로 
	// 원본 맵에서 금괴위치 말고는 EMPTY로 초기화
	for (int y = 0; y < cMapData.height; y++) {
		for (int x = 0; x < cMapData.width; x++) {
			if (mapData[currentLevel].map[y][x] == GOLD)
				cMapData.map[y][x] = mapData[currentLevel].map[y][x];
			else
				cMapData.map[y][x] = EMPTY;
		}
	}
	cMapData.playerInitPos = mapData[currentLevel].playerInitPos; // 플레이어 초기 위치 설정
	cPos = cMapData.playerInitPos; // 플레이어의 현재 위치를 초기 위치로 설정

	system("clear"); // 화면 비우기
	Render(); // 맵 출력
	return 1;
}


int Clear() {
	for (int y = 0; y < cMapData.height; y++) {
		for (int x = 0; x < cMapData.width; x++) {
		    	// 금괴 중 1개라도 저장소 위치에 있지 않다면 0 반환
			if (cMapData.map[y][x] == GOLD && mapData[currentLevel].map[y][x] != STORAGE)
				return 0;
		}
	}
	return 1;
}


int Len(char *s) {
	// 문자열의 인덱스를 0부터 확인하면서 '\0'가 아닐 때까지 i를 1씩 더함
	int i = 0;
	while (s[i] != '\0') {
		i++;
	}
	return i;

}

int IsInMap(POSITION _pos) {
    	// _pos의 x, y 좌표가 유효한 위치에 있지 않다면 0을 반환
	if (_pos.x < 0 || _pos.y < 0 || _pos.y >= mapData[currentLevel].height || _pos.x >= mapData[currentLevel].width)
		return 0;
	return 1;
}


void gotoxy(int x, int y) {
	printf("\033[%d;%df", y, x);		// 터미널 상에서 x, y좌표로 커서를 이동
	fflush(stdout);				// 출력 버퍼를 비움
}

int getch(){
    int c;
    struct termios oldattr,newattr;

    tcgetattr(STDIN_FILENO,&oldattr);		// 현재 터미널 설정 읽음
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO); 	// CANONICAL과 ECHO 끔
    newattr.c_cc[VMIN] = 1;			// 최소 입력 문자 수를 1로 설정
    newattr.c_cc[VTIME] = 0;			// 최소 읽기 대기 시간을 0으로 설정
    tcsetattr(STDIN_FILENO,TCSANOW,&newattr);	// 터미널에 설정 입력
    c = getchar();				// 키보드 입력 읽음
    tcsetattr(STDIN_FILENO,TCSANOW,&oldattr);	// 원래의 설정으로 복구
    return c;
}
