#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

#define PORT 8888

int print_out_key(void *cls, enum MHD_ValueKind kind, const char *key,
		const char *value)
{
	printf("%s: %s\n", key, value);
	return MHD_YES;
}

int answer_to_connection(void *cls, struct MHD_Connection *connection,
		const char *url, const char *method, const char *version,
		const char *upload_data, size_t *upload_data_size, void **con_cls)
{
	//const char *page = "<html><body>Hello, browser!</body></html>";
	const char *nocando = "Error man\n";
	struct MHD_Response *response;
	int ret;

	MHD_get_connection_values(connection, MHD_HEADER_KIND, &print_out_key,
	NULL);

	printf("URL: %s\nMethod: %s\nVersion: %s\n\n", url, method, version);

	if (strcmp(method, "GET") == 0)
	{
		response = MHD_create_response_from_buffer(strlen(url), (void*) url,
				MHD_RESPMEM_PERSISTENT);

		ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	}
	else
	{
		response = MHD_create_response_from_buffer(strlen(nocando), (void*) nocando,
				MHD_RESPMEM_PERSISTENT);

		ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
	}

	MHD_destroy_response(response);

	return ret;
}

int main()
{

	struct MHD_Daemon *daemon;

	daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
			&answer_to_connection, NULL, MHD_OPTION_END);
	if (NULL == daemon)
		return 1;

	getchar();

	MHD_stop_daemon(daemon);
	return 0;

}
