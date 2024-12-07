#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

#define PORT 8888
#define SYS_get_memory_info 456 // Número de syscall para obtener información de memoria
#define SYS_get_swap_info 457   // Número de syscall para obtener información de swap
#define SYS_get_page_faults 458 // Número de syscall para obtener fallos de página
#define SYS_get_active_inactive_pages 459 // Número de syscall para obtener páginas activas e inactivas
#define SYS_get_top_memory_processes 460 // Número de syscall para obtener los procesos que más memoria utilizan

// Function to get memory info using the custom syscall
char* get_memory_info() {
    char *buffer = malloc(1024);
    if (!buffer) return strdup("ERROR: Unable to allocate memory");

    if (syscall(SYS_get_memory_info, buffer) < 0) {
        free(buffer);
        return strdup("ERROR: Syscall failed");
    }

    return buffer;
}

// Function to get swap info using the custom syscall
char* get_swap_info() {
    char *buffer = malloc(256);
    if (!buffer) return strdup("ERROR: Unable to allocate memory");

    if (syscall(SYS_get_swap_info, buffer) < 0) {
        free(buffer);
        return strdup("ERROR: Syscall failed");
    }

    return buffer;
}

// Function to get page faults info using the custom syscall
char* get_page_faults_info() {
    char *buffer = malloc(256);
    if (!buffer) return strdup("ERROR: Unable to allocate memory");

    if (syscall(SYS_get_page_faults, buffer) < 0) {
        free(buffer);
        return strdup("ERROR: Syscall failed");
    }

    return buffer;
}

// Function to get active and inactive pages info using the custom syscall
char* get_active_inactive_pages_info() {
    char *buffer = malloc(256);
    if (!buffer) return strdup("ERROR: Unable to allocate memory");

    if (syscall(SYS_get_active_inactive_pages, buffer) < 0) {
        free(buffer);
        return strdup("ERROR: Syscall failed");
    }

    return buffer;
}

// Function to get top memory consuming processes info using the custom syscall
char* get_top_memory_processes_info() {
    char *buffer = malloc(1024);
    if (!buffer) return strdup("ERROR: Unable to allocate memory");

    if (syscall(SYS_get_top_memory_processes, buffer) < 0) {
        free(buffer);
        return strdup("ERROR: Syscall failed");
    }

    return buffer;
}

// Function to add CORS headers to the response
void add_cors_headers(struct MHD_Response *response) {
    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
}

// Callback function to handle requests
int answer_to_connection(void *cls, struct MHD_Connection *connection, 
                         const char *url, const char *method, 
                         const char *version, const char *upload_data, 
                         size_t *upload_data_size, void **con_cls) {
    char *page;
    struct MHD_Response *response;
    int ret;

    if (strcmp(method, "OPTIONS") == 0) {
        response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
        add_cors_headers(response);
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    if (strcmp(url, "/memory-usage") == 0) {
        page = get_memory_info();
    } else if (strcmp(url, "/swap-info") == 0) {
        page = get_swap_info();
    } else if (strcmp(url, "/page-faults") == 0) {
        page = get_page_faults_info();
    } else if (strcmp(url, "/active-inactive-pages") == 0) {
        page = get_active_inactive_pages_info();
    } else if (strcmp(url, "/top-memory-processes") == 0) {
        page = get_top_memory_processes_info();
    } else {
        page = strdup("404 Not Found");
    }

    response = MHD_create_response_from_buffer(strlen(page), (void*) page, MHD_RESPMEM_MUST_FREE);
    add_cors_headers(response);
    ret = MHD_queue_response(connection, (strcmp(url, "/memory-usage") == 0 || strcmp(url, "/swap-info") == 0 || strcmp(url, "/page-faults") == 0 || strcmp(url, "/active-inactive-pages") == 0 || strcmp(url, "/top-memory-processes") == 0) ? MHD_HTTP_OK : MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);

    return ret;
}

int main() {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL, 
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) return 1;

    printf("Server running on port %d\n", PORT);
    getchar(); // Wait for user input to stop the server

    MHD_stop_daemon(daemon);
    return 0;
}
