#include "FreeRTOS.h"
#include "task.h"
#include "basic_io.h"
#include "semphr.h"
#include "time.h"

SemaphoreHandle_t semaphore;
SemaphoreHandle_t mutex;

const TickType_t ONE_SEC_DELAY = 1000 / portTICK_PERIOD_MS; // 1 segundo baseado na taxa de ticks da kernel

struct Date {
	uint32_t day;
	uint32_t month;
	uint32_t ano;
};

struct Time {
	uint32_t second;
	uint32_t minute;
	uint32_t hour;
};

char display[50] = "";

void task1(void* parameters);
void task2(void* parameters);
void task3(void* parameters);

int main_() {
	mutex = xSemaphoreCreateMutex();
	srand(21);

	xTaskCreate(task1, "task1", 10, 1, 1, NULL);
	xTaskCreate(task2, "task2", 10, 2, 1, NULL);
	xTaskCreate(task3, "task3", 10, 3, 3, NULL);

	vTaskStartScheduler();
	for (;;);
	return 0;
}

static float generate_random_float(float max) {
	float n = ((float)rand() / (float)RAND_MAX) * max;
	return n;
}

void task1(void* parameters) {
	struct Date current_date = { 0,0,0 }; // instanciamos uma struct vazia para representar a data
	int id = (int)parameters; // pegamos o id passado como parametro
	time_t seconds = 0; // instanciamos o valor para segundos
	for (;;) {
		time(&seconds); // passamos a referencia do segundos para setar o tempo atual em segundos
		struct tm* time_now = localtime(&seconds); // convertemos o segundos para um outro struct que representa o dia atual
		// pegamos todos os valores e colocamos dentro da struct que fizemos
		current_date.day = time_now->tm_mday;
		current_date.month = time_now->tm_mon + 1;
		current_date.ano = time_now->tm_year + 1900;
		// damos take no semaforo para verificar sua disponibilidade
		// se nao tiver e o delay de espera para tentar de novo passou, reinicia o for, se tiver, pega o buffer display e cria uma nova string formata com os dados acima
		if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
			sprintf(display, "Task %ld - %d/%d/%d\n", id, current_date.day, current_date.month, current_date.ano);
			vPrintString(display);
			xSemaphoreGive(mutex); // damo give no mutex, liberando o buffer para o uso em outras tasks
			vTaskDelay(ONE_SEC_DELAY); // damos delay para ficar facil de observar
		}
	}
	vTaskDelete(NULL);
}

// faz a mesma coisa que a task1 acima, mas ao inves de pegar dia, mes e ano, pega hora, minuto e segundo
void task2(void* parameters) {
	struct Time current_time = { 0,0,0 };
	int id = (int)parameters;
	time_t seconds = 0;
	for (;;) {
		time(&seconds);
		struct tm* time_now = localtime(&seconds);
		current_time.hour = time_now->tm_hour;
		current_time.minute = time_now->tm_min;
		current_time.second = time_now->tm_sec;
		if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
			sprintf(display, "Task %ld - %d:%d:%d\n", id, current_time.hour, current_time.minute, current_time.second);
			vPrintString(display);
			xSemaphoreGive(mutex);
			vTaskDelay(ONE_SEC_DELAY);
		}
	}
	vTaskDelete(NULL);
}

void task3(void* parameters) {
	int id = (int)parameters;
	for (;;) {
		float temp = generate_random_float(30); // geramos um float aleatorio com valor maximo de 30
		if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) { // damos take para usarmos o buffer global
			sprintf(display, "Task %ld - Curitiba %.2f C\n", id, temp);
			vPrintString(display);
			xSemaphoreGive(mutex); // liberamos o buffer para o uso em outras tasks
			vTaskDelay(ONE_SEC_DELAY);
		}
	}
	vTaskDelete(NULL);
}

