#include <stdio.h>
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

// 위치 정보
typedef struct _tagPosition {
	int x;						
	int	y;
}POSITION, *PPOSITION;

// 맵의 정보
typedef struct _tagMapData {
	int map[MAPSIZE][MAPSIZE];	// 맵 정보를 저장하는 배열
	int width, height;			// 맵의 높이와 길이
	POSITION playerInitPos;		// 플레이어의 시작점
}MAPDATA, *PMAPDATA;


POSITION	cPos;				// 캐릭터의 현재 위치

MAPDATA		mapData[NUMBER_OF_MAPS];		// File로 부터 받아온 맵 정보

int Len(char *s);				// 문자열의 길이를 출력하는 함수
int MapLoading();				// map파일로부터 맵을 로딩하는 함수

int main(){
	if (!MapLoading()) {
		fprintf(stderr,"MapLoading() Error\n");
		return 0;
	}
	return 0;
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

int Len(char *s) {
	// 문자열의 인덱스를 0부터 확인하면서 '\0'가 아닐 때까지 i를 1씩 더함
	int i = 0;
	while (s[i] != '\0') {
		i++;
	}
	return i;
}