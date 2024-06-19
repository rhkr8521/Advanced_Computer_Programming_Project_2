#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h> // 윈도우의 키보드 입력을 처리하기 위한 헤더 파일

typedef struct { // 커서 위치 저장 구조체
    int x;
    int y;
} Position;

typedef struct {
    char nickname[100];
    double time_elapsed;
    char datetime[26];
} Record;

int** pan;
int** revealed;
int** flagged;

int mainpage();

Position pos = { 0, 0 }; // 초기 위치를 (0,0)으로 설정

char which(int what) {
    if (what == 0)
        return '.';
    if (what >= 1 && what <= 8)
        return '0' + what; // Convert number to character
    if (what == 9)
        return ' ';
    if (what == -1)
        return '*';
    return ' ';
}

// 맵 배치
void mapBoom(int size, int boom) {
    srand((unsigned int)time(NULL));

    for (int i = 0; i < boom; i++) {
        int x = rand() % size;
        int y = rand() % size;

        if (pan[y][x] == -1) {
            i--; // 동일한 지뢰가 있는 경우 다시 반복
        }
        else {
            pan[y][x] = -1; // 지뢰 할당
        }
    }
}

// 주변에 폭탄이 몇 개 있는지 체크
int checkBoom(int x, int y, int size) {
    if (pan[y][x] == -1)
        return -1; // 지뢰가 있는 경우

    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (y + i >= 0 && y + i < size && x + j >= 0 && x + j < size) {
                if (pan[y + i][x + j] == -1) count++;
            }
        }
    }

    return count == 0 ? 9 : count;
}

// 인접한 빈 공간을 재귀적으로 열기
void reveal(int x, int y, int size) {
    if (x < 0 || x >= size || y < 0 || y >= size || revealed[y][x] || flagged[y][x])
        return;

    revealed[y][x] = 1;
    int result = checkBoom(x, y, size);
    if (result != -1) {
        pan[y][x] = result;
    }

    if (result == 9) { // 주변에 폭탄이 없으면 인접 칸도 열기
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i != 0 || j != 0)
                    reveal(x + i, y + j, size);
            }
        }
    }
}

// 맵 프린트
void print(int size, int remaining_mines) {
    system("cls"); // 윈도우에서 터미널 화면을 지움
    printf("남은 지뢰 갯수: %d\n\n", remaining_mines);

    printf("   ");
    for (int cx = 0; cx < size; cx++) {
        printf("%2d ", cx + 1);
    }
    printf("\n");

    for (int i = 0; i < size; i++) {
        printf("%2d ", i + 1);
        for (int j = 0; j < size; j++) {
            if (i == pos.y && j == pos.x) // 현재 for문의 i, j가 플레이어의 pos.y, pos.x와 일치하면 그 위치에 P를 출력
                printf("%2c ", 'P');
            else if (flagged[i][j])
                printf("%2c ", '#');
            else if (revealed[i][j])
                printf("%2c ", which(pan[i][j]));
            else
                printf(" . ");
        }
        printf("\n");
    }
}

// 게임이 이겼는지 확인
int checkWin(int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (pan[i][j] != -1 && !revealed[i][j]) {
                return 0; // 지뢰가 아닌데 아직 열리지 않은 칸이 있음
            }
        }
    }
    return 1; // 모든 지뢰가 아닌 칸이 열림
}

// 기록 저장
void saveRecord(char* nickname, time_t start_time, time_t end_time, int difficulty) {
    FILE* fp;
    if (difficulty == 1)
        fp = fopen("minesweeper_record_easy.txt", "a");
    else if (difficulty == 2)
        fp = fopen("minesweeper_record_medium.txt", "a");
    else
        fp = fopen("minesweeper_record_hard.txt", "a");

    if (fp == NULL) {
        printf("파일을 열 수 없습니다.\n");
        return;
    }

    struct tm* tm_info = localtime(&end_time);
    char buffer[26];
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    double elapsed = difftime(end_time, start_time);

    fprintf(fp, "날짜: %s, 닉네임: %s, 기록: %.2f seconds\n", buffer, nickname, elapsed);
    fflush(fp); // 파일 버퍼를 비워 실제 파일에 즉시 기록하도록 함
    fclose(fp);
}

int compare(const void* a, const void* b) {
    Record* record_a = (Record*)a;
    Record* record_b = (Record*)b;
    return (record_a->time_elapsed > record_b->time_elapsed) - (record_a->time_elapsed < record_b->time_elapsed);
}

void showRecords(int difficulty) {
    FILE* fp;
    if (difficulty == 1)
        fp = fopen("minesweeper_record_easy.txt", "r");
    else if (difficulty == 2)
        fp = fopen("minesweeper_record_medium.txt", "r");
    else
        fp = fopen("minesweeper_record_hard.txt", "r");

    if (fp == NULL) {
        printf("기록 파일을 열 수 없습니다.\n");
        return;
    }

    Record records[100];
    int num_records = 0;
    char line[256]; // 한 줄씩 읽기 위한 버퍼
    while (fgets(line, sizeof(line), fp) != NULL) { // 한 줄씩 읽기
        if (sscanf(line, "날짜: %25[^,], 닉네임: %99[^,], 기록: %lf", records[num_records].datetime, records[num_records].nickname, &records[num_records].time_elapsed) == 3) {
            num_records++;
            if (num_records >= 100) break; // 배열 크기를 넘지 않도록 보호
        }
    }
    fclose(fp);

    qsort(records, num_records, sizeof(Record), compare);

    for (int i = 0; i < num_records; i++) {
        printf("날짜: %s, 닉네임: %s, 기록: %.2f seconds\n", records[i].datetime, records[i].nickname, records[i].time_elapsed);
    }
}

// 방향키를 읽는 함수
char read_key() {
    int ch = _getch();
    if (ch == 0 || ch == 224) {
        // Special key prefix
        ch = _getch();
        switch (ch) {
        case 72:
            return 'A'; // 위쪽 방향키
        case 80:
            return 'B'; // 아래쪽 방향키
        case 77:
            return 'C'; // 오른쪽 방향키
        case 75:
            return 'D'; // 왼쪽 방향키
        }
    }
    return ch;
}

void difficulty(int num, int* size, int* boom) {
    if (num == 1) {
        *size = 9;
        *boom = 1;      // text용 level
    }
    else if (num == 2) {
        *size = 16;
        *boom = 30;
    }
    else if (num == 3) {
        *size = 28;
        *boom = 70;
    }
}

int mainPage_Up_game() { //게임 선택 모드
    int select_LV = 0;
    system("cls");
    printf("-------------------------------------------\n");
    printf("\n");
    printf("           방구뿡뿡띠 지뢰찾기!            \n");
    printf("\n");
    printf("-------------------------------------------\n");
    printf("\n");
    printf("--난이도--\n");
    printf("1. 초보자 모드\n");
    printf("2. 중급자 모드\n");
    printf("3. 상급자 모드\n");
    printf("4. 뒤로\n");
    printf("\n");

    printf("선택 : ");
    scanf("%d", &select_LV);
    if (select_LV != 1 && select_LV != 2 && select_LV != 3 && select_LV != 4) {
        mainPage_Up_game();
    }
    else if (select_LV == 4) {
        mainpage();
    }
    else {
        return select_LV;
    }
}

int recordPage() {
    int select_LV = 0;
    system("cls");
    printf("-------------------------------------------\n");
    printf("\n");
    printf("           방구뿡뿡띠 지뢰찾기!            \n");
    printf("\n");
    printf("-------------------------------------------\n");
    printf("\n");
    printf("--기록보기--\n");
    printf("1. 초보자 모드 기록\n");
    printf("2. 중급자 모드 기록\n");
    printf("3. 상급자 모드 기록\n");
    printf("4. 뒤로\n");
    printf("\n");

    printf("선택 : ");
    scanf("%d", &select_LV);
    if (select_LV != 1 && select_LV != 2 && select_LV != 3 && select_LV != 4) {
        recordPage();
    }
    else if (select_LV == 4) {
        mainpage();
    }
    else {
        return select_LV;
    }
}

int mainpage() {
    int select_function = 0;
    system("cls");
    printf("-------------------------------------------\n");
    printf("\n");
    printf("           방구뿡뿡띠 지뢰찾기!            \n");
    printf("\n");
    printf("-------------------------------------------\n");
    printf("\n");
    printf("1. 게임 시작\n");
    printf("2. 기록 확인\n");
    printf("3. 종료\n");
    printf("\n");

    printf("선택 : ");
    scanf("%d", &select_function);
    if (select_function != 1 && select_function != 2 && select_function != 3) {
        mainpage();
    }
    else if (select_function == 1) {
        mainPage_Up_game();
    }
    else if (select_function == 2) {
        int lv = recordPage();
        if (lv == 4) return 0;
        showRecords(lv);
        return 0;
    }
    else if (select_function == 3) {
        printf("종료합니다.\n");
        return 0;
    }
    else {
        return select_function;
    }
}

int main() {
    int size, boom;
    int num;
    char nickname[100];

    num = mainpage(); //select_
    if (num == 0) {
        return 0;
    }

    difficulty(num, &size, &boom);

    pan = (int**)malloc(sizeof(int*) * size);
    revealed = (int**)malloc(sizeof(int*) * size);
    flagged = (int**)malloc(sizeof(int*) * size);

    for (int i = 0; i < size; i++) {
        pan[i] = (int*)malloc(sizeof(int) * size);
        revealed[i] = (int*)malloc(sizeof(int) * size);
        flagged[i] = (int*)malloc(sizeof(int) * size);
        memset(pan[i], 0, sizeof(int) * size); // 초기화
        memset(revealed[i], 0, sizeof(int) * size);
        memset(flagged[i], 0, sizeof(int) * size);
    }

    mapBoom(size, boom);

    time_t start_time, end_time;
    time(&start_time);

    int remaining_mines = boom;

    printf("\n");
    printf("닉네임을 입력하세요: ");
    getchar(); // 이전 입력 버퍼에 남아있는 개행 문자를 처리하기 위해 추가
    fgets(nickname, sizeof(nickname), stdin);
    nickname[strcspn(nickname, "\n")] = 0;

    while (1) {
        print(size, remaining_mines);

        char c = read_key();

        if (c == 'A') { // 위쪽 방향키
            if (pos.y > 0) pos.y--;
        }
        else if (c == 'B') { // 아래쪽 방향키
            if (pos.y < size - 1) pos.y++;
        }
        else if (c == 'C') { // 오른쪽 방향키
            if (pos.x < size - 1) pos.x++;
        }
        else if (c == 'D') { // 왼쪽 방향키
            if (pos.x > 0) pos.x--;
        }
        else if (c == 'f') { // 'f' 키를 누르면 플래그를 세움
            if (!revealed[pos.y][pos.x]) {
                flagged[pos.y][pos.x] = !flagged[pos.y][pos.x];
                remaining_mines += flagged[pos.y][pos.x] ? -1 : 1;
            }
        }
        else if (c == '\n' || c == '\r') { // 엔터 키를 누르면 패를 염
            if (pan[pos.y][pos.x] == -1) {
                for (int i = 0; i < size; i++) {
                    for (int j = 0; j < size; j++) {
                        if (pan[i][j] == -1)
                            revealed[i][j] = 1;
                    }
                }
                printf("\n");
                print(size, remaining_mines);
                printf("지뢰를 클릭했습니다! 게임 종료.\n\n");

                break;
            }
            else {
                reveal(pos.x, pos.y, size);
                if (checkWin(size)) {
                    time(&end_time);
                    saveRecord(nickname, start_time, end_time, num);
                    print(size, remaining_mines);
                    printf("\n");
                    printf("이겼습니다!\n\n");
                    break;
                }
            }
        }

        print(size, remaining_mines);
    }
    printf("\n");
    printf("기록을 보시겠습니까? (1: 예, 2: 아니오): ");
    int show;
    scanf("%d", &show);
    if (show == 1) {
        showRecords(num);
    }

    for (int i = 0; i < size; i++) {
        free(pan[i]);
        free(revealed[i]);
        free(flagged[i]);
    }
    free(pan);
    free(revealed);
    free(flagged);

    return 0;
}
