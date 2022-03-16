/*
 ============================================================================
 Name        : CAN_convert_data.c
 Author      : Leo
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : Convert can data to raw data
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/can.h>
#include <time.h>
#include <unistd.h>		// sleep() function

#define CHAR_PER_LINE	150

struct dbc_data {
	canid_t can_id;
	float offset;
	int scale;
	char unit[20];
	char signal[20];
	int start_bit;
	int stop_bit;
};

struct can_data {
	unsigned long long timestamp;
	canid_t can_id;
	unsigned int data[8] __attribute__((aligned(8)));
};

void convert_can_data(struct can_data *data_frame, struct dbc_data*, int *count);
void load_dbc_data(struct dbc_data*);
int id_count_dbc_data();

int main(void) {

	/* struct to hold temporary can_data */
	struct can_data data_frame;

	/* count canid in csv file */
	int id_count = id_count_dbc_data();

	/* read dbc-data from csv file. Create array of structs */
	struct dbc_data dbc_array[id_count];
	load_dbc_data(dbc_array);

	while (1) {
		FILE *fp;
		if ((fp = fopen("can_data.csv", "r")) != NULL) {
			// file exists

			/* antal karaktere der kan være på en linje */
			char line[CHAR_PER_LINE];

			/* read until end of can_data file */
			while (!feof(fp)) {

				/* read a line in file */
				fgets(line, CHAR_PER_LINE, fp);

				/* data from file into struct */
				sscanf(line,
						"%llu, %05X, %03X %03X %03X %03X %03X %03X %03X %03X",
						&data_frame.timestamp, &data_frame.can_id,
						&data_frame.data[0], &data_frame.data[1],
						&data_frame.data[2], &data_frame.data[3],
						&data_frame.data[4], &data_frame.data[5],
						&data_frame.data[6], &data_frame.data[7]);

				/* covert can data function */
				convert_can_data(&data_frame, dbc_array, &id_count);

			}

			fclose(fp);

			/* delete file */
			remove("can_data.csv");

		} else {
			//File not found, no memory leak since 'file' == NULL
			//fclose(file) would cause an error
			printf("No file\n");
			sleep(1);
		}
	}
	return EXIT_SUCCESS;
}

int id_count_dbc_data() {
	/*
	 * read dbc-data from text file. Create array of structs
	 */

	FILE *pfile;
	pfile = fopen("DBC.csv", "r");

	if (pfile == NULL) {
		perror("file_open_read");
	}

	int count = 0;

	/* antal karaktere der kan være på en linje */
	char line[CHAR_PER_LINE];

	fgets(line, CHAR_PER_LINE, pfile); // læs føsrte linje i fil, skal ikke bruges til noget

	while (!feof(pfile)) {
		fgets(line, CHAR_PER_LINE, pfile); // læs en linje i fil
		count++;
	}

	fclose(pfile);
	return count;
}

void load_dbc_data(struct dbc_data *ptr) {
	/*
	 * read dbc-data from csv/text file. Create array of structs
	 */

	FILE *pfile;
	pfile = fopen("DBC.csv", "r");

	if (pfile == NULL) {
		perror("file_open_read");
	}

	/* antal karaktere der kan være på en linje */
	char line[CHAR_PER_LINE];

	fgets(line, CHAR_PER_LINE, pfile); // læs føsrte linje i fil, skal ikke bruges til noget

	while (!feof(pfile)) {
		fgets(line, CHAR_PER_LINE, pfile); // læs en linje i fil

		sscanf(line, "%03X, %f, %d, %[^,], %[^,], %d, %d", &ptr->can_id,
				&ptr->offset, &ptr->scale, ptr->unit, ptr->signal,
				&ptr->start_bit, &ptr->stop_bit);
		ptr++;
	}

	fclose(pfile);

}

void convert_can_data(struct can_data *data_frame, struct dbc_data *dbc_array,
		int *count) {

	//int count1 = *count;

	int i;
	for (i = 0; i < *count; i++) {

		if (data_frame->can_id == dbc_array->can_id) {
			/* casting hex to decimal */
			uint64_t raw_value_decimal = 0;

			int num = 0;
			int ii;
			for (ii = dbc_array->start_bit; ii < dbc_array->stop_bit + 1;
					ii++) {
				raw_value_decimal |= data_frame->data[ii] << num * 8;
				num++;
			}

			/* timestamp to reable clock/time  */
			int ms = data_frame->timestamp%1000;
			int sec = data_frame->timestamp/1000;

			time_t time = sec;

			struct tm timeinfo;
			timeinfo = *localtime(&time);
			char buf[80];

			strftime(buf, sizeof(buf), "%A %d %B %Y %H:%M:%S", &timeinfo);

			float physical_value = dbc_array->offset
					+ dbc_array->scale * raw_value_decimal;
			
			/* print data */
			printf("%s.%d	%s: %.3f %s\n", buf, ms, dbc_array->signal,
					physical_value, dbc_array->unit);

			printf("\n");

		}

		/* increment pointer in array */
		dbc_array++;
	}
}
