#include <iostream>
#include <mpi.h>
#include <chrono>
#include <vector>
#include <cstdlib>

// Константы для размера сетки и количества итераций
const int ROWS = 96;
const int COLS = 1000;
const int ITERATIONS = 1000;

/**
 * инициализация карты случайными элементами
 *
 * @param map карта,2 которую нужно заполнить
 */
void initializeMap(std::vector<std::vector<int>>& map);

/**
 * Обновляет ячейки сетки в соответствии с правилами игры "Жизнь".
 *
 * @param map       текущая карта
 * @param nextGrid  следующая ячейка для исследования
 * @param startRow  с какой строк начинать для текущего процесса
 * @param endRow    на какой строек остановиться для текущего процесса
 */
void updateCells(
        const std::vector<std::vector<int>>& map,
        std::vector<std::vector<int>>& nextGrid,
        int startRow,
        int endRow
);

/**
 * Обменивается граничными строками с соседними процессами для правильного обновления ячеек на границах.
 *
 * @param map       текущая карта состояний
 * @param rank      ранг текущего процесса
 * @param size      общее кол-во процессов
 * @param startRow  с какой строки начать для текущего процесса
 * @param endRow    какой строкой закончить для текущего процесса
 */
void exchangeBoundaryRows(
        std::vector<std::vector<int>>& map,
        int rank,
        int size,
        int startRow,
        int endRow
);

/**
 * Функция подсчета живых клеток
 *
 * @param map       карта игры в жизнь
 * @param startRow  с какой строки начинать
 * @param endRow    какой строкой закончить
 *
 * @return кол-во живых клеток
 */
int countAliveCells(
        const std::vector<std::vector<int>>& map,
        int startRow,
        int endRow
);

int main(int argc, char** argv) {
    // Инициализация MPI
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Получение ранга текущего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Получение общего количества процессов

    // Инициализация сетки и сетки следующего состояния
    std::vector<std::vector<int>> grid(ROWS, std::vector<int>(COLS));
    std::vector<std::vector<int>> nextGrid(ROWS, std::vector<int>(COLS));
    initializeMap(grid);

    // Определение начальной и конечной строки для каждого процесса
    int startRow = rank * (ROWS / size);
    int endRow = (rank + 1) * (ROWS / size) - 1;

    auto startTime = std::chrono::system_clock::now();

    // эволюция
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        updateCells(grid, nextGrid, startRow, endRow);
        exchangeBoundaryRows(grid, rank, size, startRow, endRow);
        std::swap(grid, nextGrid);
    }

    // Сбор результатов
    if (rank == 0) {
        MPI_Status status;
        for (int i = 1; i < size; ++i) {
            for (int j = 0; j < ROWS / size; ++j) {
                int row = i * ROWS / size + j;
                MPI_Recv(grid[row].data(), COLS, MPI_INT, i, row, MPI_COMM_WORLD, &status); // Получение строк от других процессов
            }
        }
        int aliveCells = countAliveCells(grid, 0, ROWS - 1); // Подсчет живых клеток
        std::cout << "Alive cells: " << aliveCells << std::endl;
    } else {
        for (int i = 0; i < ROWS / size; ++i) {
            int row = startRow + i;
            MPI_Send(grid[row].data(), COLS, MPI_INT, 0, row, MPI_COMM_WORLD); // Отправка строк процессу 0
        }
    }

    auto endTime = std::chrono::system_clock::now(); // Остановка таймера
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    std::cout << "Elapsed time: " << elapsedSeconds.count() << " sec" << " rank " << rank << std::endl; // Вывод времени выполнения

    MPI_Finalize();
    return 0;
}

// Функция инициализации сетки случайными значениями
void initializeMap(std::vector<std::vector<int>>& map) {
    srand(42); // Фиксированный seed для генератора случайных чисел
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            map[i][j] = rand() % 2; // Заполнение сетки случайными 0 и 1
        }
    }
}

// Функция обновления ячеек сетки
void updateCells(
        const std::vector<std::vector<int>>& map,
        std::vector<std::vector<int>>& nextGrid,
        int startRow,
        int endRow
) {
    for (int i = startRow; i <= endRow; ++i) {
        for (int j = 0; j < COLS; ++j) {
            int neighbors = 0;
            // Подсчет соседей
            for (int x = std::max(0, i-1); x <= std::min(ROWS-1, i+1); ++x) {
                for (int y = std::max(0, j-1); y <= std::min(COLS-1, j+1); ++y) {
                    if (x != i || y != j) {
                        neighbors += map[x][y];
                    }
                }
            }
            // Правила игры "Жизнь"
            if (map[i][j] == 1 && neighbors < 2) {
                nextGrid[i][j] = 0;
            } else if (map[i][j] == 1 && (neighbors == 2 || neighbors == 3)) {
                nextGrid[i][j] = 1;
            } else if (map[i][j] == 1 && neighbors > 3) {
                nextGrid[i][j] = 0;
            } else if (map[i][j] == 0 && neighbors == 3) {
                nextGrid[i][j] = 1;
            } else {
                nextGrid[i][j] = 0;
            }
        }
    }
}

// Функция обмена граничными строками между процессами
void exchangeBoundaryRows(
        std::vector<std::vector<int>>& map,
        int rank,
        int size,
        int startRow,
        int endRow
) {
    int topProccess = (rank + size - 1) % size;
    int bottomProcess = (rank + 1) % size;

    int rowAboveMine = (startRow + ROWS - 1) % ROWS;
    int rowAfterMine = (endRow + 1) % ROWS;

    MPI_Status status;

    // Чередование отправки/получения строк в зависимости от ранга процесса
    if (rank % 2 == 0) {
        MPI_Sendrecv(map[startRow].data(), COLS, MPI_INT, topProccess, rank,
                     map[rowAboveMine].data(), COLS, MPI_INT, topProccess, topProccess,
                     MPI_COMM_WORLD, &status);

        MPI_Sendrecv(map[endRow].data(), COLS, MPI_INT, bottomProcess, rank,
                     map[rowAfterMine].data(), COLS, MPI_INT, bottomProcess, bottomProcess,
                     MPI_COMM_WORLD, &status);
    } else {
        MPI_Sendrecv(map[endRow].data(), COLS, MPI_INT, bottomProcess, rank,
                     map[rowAfterMine].data(), COLS, MPI_INT, bottomProcess, bottomProcess,
                     MPI_COMM_WORLD, &status);

        MPI_Sendrecv(map[startRow].data(), COLS, MPI_INT, topProccess, rank,
                     map[rowAboveMine].data(), COLS, MPI_INT, topProccess, topProccess,
                     MPI_COMM_WORLD, &status);
    }
}

// Функция подсчета живых клеток
int countAliveCells(const std::vector<std::vector<int>>& map, int startRow, int endRow) {
    int alive = 0;
    for (int i = startRow; i <= endRow; ++i) {
        for (int j = 0; j < COLS; ++j) {
            alive += map[i][j];
        }
    }
    return alive;
}
