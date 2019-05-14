#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <io.h>
#include <string.h>

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
#define ENTER 13

#define WALL 35
#define EMPTY 46
#define GOLD 36
#define PLAYER 64
#define STORAGE 79

#define NUMBER_OF_MAPS 5

#define MAPSIZE 31
typedef struct _tagPosition {
	int x;
	int	y;
}POSITION, *PPOSITION;

typedef struct _tagMapData {
	int map[MAPSIZE][MAPSIZE];
	int width, height;
	POSITION playerInitPos;

}MAPDATA, *PMAPDATA;

typedef struct _tagMoveInfo {
	POSITION delta;
	POSITION goldPosition;
}MOVEINFO, *PMOVEINFO;

typedef struct _tagRankingInfo {
	char name[10];
	int moveCount;
}RANKING, *PRANKING;

void gotoxy(int x, int y);
int Len(char *s);
int MapLoading(); 
int SetMap(int level);
int IsInMap(POSITION _pos);
void Input();
void Render();
void Move(int delX,int delY,int undoMoving); // x������ delX��ŭ, y�� delY ��ŭ
void Undo();
void New();
void DisplayHelp();
int Save();
int FileLoad();
int RankingSave();
int RankingLoad();
void RankingDisplay();
int Clear();

// ����Ǿ���� ����
char name[10];				// �̸�
int movingCount;			// ������ ����
int undoCount = 5;		// undo�� �� �ִ� Ƚ��
int currentLevel = 0;		// ���� ���� ���� 0 ~ 4
MAPDATA cMapData;			// ������ map �������� ���� ��ġ�� ǥ��
POSITION cPos;				// ĳ������ ���� ��ġ
MOVEINFO moveInfo[5];		// �ֱ� 5���� �̵��� ���� ����

// ������� �ʾƵ� �Ǵ� ����
MAPDATA mapData[NUMBER_OF_MAPS];			// File�� ���� �޾ƿ� �� ����
int topPressedBeforeFrame;	// ������ TOPŰ�� ���ȴ���
int showTopLevel = 0;		// ��ŷ�� �� ���� ����
int isPlay = 1;	
RANKING rankingList[NUMBER_OF_MAPS][5]; // [����][5������][�̸�, �̵� ��]


/*
- h(����), j(�Ʒ�), k(��), l(������) : ���� �Ʒ� �� ������ â������ ����
- u(undo) : 5 �ִ� �� �� �� ����
- r(replay) : ( ) ���� ���� ó������ �ٽ� ���� ������ Ƚ���� ��� ����
- n(new) : ( ) ù ��° �ʺ��� �ٽ� ���� ������ Ƚ�� ��� ����
- e(exit) : . ���� ���� �����ϱ� �� �ʿ��� ���� �����ؾ� ��
- s(save) : . sokoban ���� ���� ���Ͽ� ���� ���� �̸��� ���� �ϰ� ������ �ٽ� ������ �����ؼ� ��
�� �ֵ��� ��� ���� �����ؾ� ��
- f(file load) : sokoban save ���Ͽ��� ����� ������ �о� ������������ �̾ �����ϰ� ��
- d(display help) : ��� ���� ������
- t(top) : . t . t ���� ���� ������ �� �Է��ϸ� ��ü ���� ���� ���ڰ� ���� �ش� ��W�� ����
*/
int main() {
	
	if (!MapLoading()) {
		fprintf(stderr,"MapLoading() Error\n");
		return 0;
	}

	printf("�̸��� �Է��Ͻʽÿ�: ");
	scanf("%s", name);

	if (!SetMap(0)) {
		fprintf(stderr, "Init SetMap() Error\n");
		return 0;
	}
	RankingLoad();
	
	while (isPlay) {
		Input();
		Render();

		if (Clear()) {

			system("cls");
			RankingSave();
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
			New();
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
	case ENTER:
		if (topPressedBeforeFrame)
		{
			RankingDisplay();
			showTopLevel = 0;
		}
	default:
		if (topPressedBeforeFrame) {
			if (c - '0' >= 1 && c - '0' <= NUMBER_OF_MAPS) {
				topPressed = 1;
				showTopLevel = c - '0';
			}
		}
		else showTopLevel = 0;

	}

	topPressedBeforeFrame = topPressed;
}

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
	printf("undoCount  : %5d", undoCount);
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
	system("cls");
	printf("h(����), j(�Ʒ�), k(��), l(������)\n");
	printf("u(undo)\n");
	printf("r(replay)\n");
	printf("n(new)\n");
	printf("e(exit)\n");
	printf("s(save)\n");
	printf("f(file load)\n");
	printf("d(display help)\n");
	printf("t(top)\n");
	getch();
	system("cls");
}

int Save() {
	FILE *sokoban;

	if((sokoban = fopen("sokoban.txt","w") ) == NULL) {
		fprintf(stderr,"sokoban.txt ������ �ҷ����� �� �߽��ϴ�.\n");
		return 0;
	}

	fprintf(sokoban, "%s\n", name);
	fprintf(sokoban, "%d\n", movingCount);
	fprintf(sokoban, "%d\n", undoCount);
	fprintf(sokoban, "%d\n", currentLevel);
	// �� ���� ����
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
		fprintf(stderr, "sokoban.txt ������ �������� �ʽ��ϴ�.\n");
		return 0;
	}

	if ((sokoban = fopen("sokoban.txt", "r")) == NULL) {
		fprintf(stderr,"sokoban.txt ������ �ҷ����� �� �߽��ϴ�.\n");
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

	system("cls");
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
		fprintf(stderr, "ranking.txt ������ �ҷ����� �� �߽��ϴ�.\n");
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
		fprintf(stderr, "ranking.txt ������ �ҷ����� �� �߽��ϴ�.\n");
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
	
	system("cls");

	if (showTopLevel == 0) {
		for (int i = 0; i < NUMBER_OF_MAPS; i++) {
			printf("map %d\n", i + 1);
			for (int j = 0; j < 5; j++) {
				if (rankingList[i][j].moveCount > 0) {
					printf("%s %d\n", rankingList[i][j].name, rankingList[i][j].moveCount);
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
		}
	}

	getch();
}

int MapLoading() {
	FILE *mapFile;

	if (access("map.txt", 0) == -1) {
		fprintf(stderr, "map.txt ������ �������� �ʽ��ϴ�.\n");
		return 0;
	}
	
	if ((mapFile = fopen("map", "r")) == 0) {
		printf("map.txt ������ �ҷ����� �� �߽��ϴ�.\n");
		return 0;
	}

	char s[MAPSIZE];		// �� �پ� �޾ƿ��� ���� ����
	int returnValue = 1;	// ���� �� 1 == �����۵�, 0 == �������۵�

	int strLength = 0;	// ���� �о�帰 ���ڿ��� ���� 
	int level = -1;		// ���� �ε��ϴ� ���� ����
	int currentY = 0;	// ���� �ε� ���� ���� Y�� ��ġ

	int goldCount = 0, storageCount = 0;
	while (fscanf(mapFile, "%s",s) > 0) {
		strLength = Len(s);
		if (strLength > 30)		{
			printf("�������� ���� map�����Դϴ�.\n");
			returnValue = 0;
			break;
		}

		if (strLength == 1) {
			if (s[0] == 'e')
				break;
			else if (s[0] - '0' > 0) {
				if (goldCount != storageCount){
					printf("%d�� ���� ���� ��������� ������ ���� �ʽ��ϴ�.\n", level );
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
					printf("�������� ���� map�����Դϴ�.\n");
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
	currentLevel = level;
	if (!(mapData[currentLevel].width > 0 && mapData[currentLevel].height > 0))
		return 0;

	cMapData.width = mapData[currentLevel].width;
	cMapData.height = mapData[currentLevel].height;
	for (int y = 0; y < cMapData.height; y++) {
		for (int x = 0; x < cMapData.width; x++) {
			if (mapData[currentLevel].map[y][x] == GOLD)
				cMapData.map[y][x] = mapData[currentLevel].map[y][x];
			else
				cMapData.map[y][x] = EMPTY;
		}
	}
	cMapData.playerInitPos = mapData[currentLevel].playerInitPos;
	cPos = cMapData.playerInitPos;

	system("cls");
	Render();
	return 1;
}

int Clear() {

	for (int y = 0; y < cMapData.height; y++) {
		for (int x = 0; x < cMapData.width; x++) {
			if (cMapData.map[y][x] == GOLD && mapData[currentLevel].map[y][x] != STORAGE)
				return 0;
		}
	}
	return 1;
}

int Len(char *s) {
	int i = 0;
	while (s[i] != '\0') {
		i++;
	}
	return i;

}

int IsInMap(POSITION _pos) {
	if (_pos.x < 0 || _pos.y < 0 || _pos.y >= mapData[currentLevel].height || _pos.x >= mapData[currentLevel].width)
		return 0;
	return 1;
}

void gotoxy(int x, int y)
{
	COORD Pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}