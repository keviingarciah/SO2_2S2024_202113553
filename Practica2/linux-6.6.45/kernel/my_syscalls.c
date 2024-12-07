#include <linux/syscalls.h> // Encabezado para definiciones de syscalls
#include <linux/fs.h> // Encabezado para operaciones del sistema de archivos
#include <linux/uaccess.h> // Encabezado para acceso a memoria de espacio de usuario
#include <linux/slab.h> // Encabezado para asignación de memoria en el kernel
#include <linux/kthread.h> // Encabezado para hilos del kernel
#include <linux/delay.h> // Encabezado para funciones de demora
#include <linux/syscalls_usac.h> // Encabezado personalizado para definiciones de syscalls

// Estructura para almacenar fragmentos de datos
typedef struct {
    unsigned char *data; // Puntero al buffer de datos
    size_t data_size; // Tamaño del buffer de datos
    unsigned char *key; // Puntero al buffer de clave
    size_t key_size; // Tamaño del buffer de clave
    size_t start; // Índice de inicio del fragmento
    size_t end; // Índice de fin del fragmento
} DataFragment;

// Estructura para almacenar argumentos de la tarea
struct task_parameters {
    DataFragment fragment; // Fragmento de datos a procesar
    struct completion done; // Estructura de finalización para señalar la finalización de la tarea
};

// Función que realiza la operación XOR en un fragmento de datos
int xor_task(void *arg) {
    struct task_parameters *params = (struct task_parameters *)arg; // Convertir argumento a task_parameters
    DataFragment *fragment = &params->fragment; // Obtener el fragmento de datos
    size_t i;

    printk(KERN_INFO "Thread started: start=%zu, end=%zu\n", fragment->start, fragment->end); // Registrar inicio del hilo

    // Realizar operación XOR en el fragmento de datos
    for (i = fragment->start; i < fragment->end; i++) {
        fragment->data[i] ^= fragment->key[i % fragment->key_size];
    }

    printk(KERN_INFO "Thread completed: start=%zu, end=%zu\n", fragment->start, fragment->end); // Registrar finalización del hilo
    complete(&params->done); // Señalar finalización de la tarea
    return 0;
}

// Función que procesa el archivo de entrada y realiza la operación XOR
int process_file(const char *input_path, const char *output_path, const char *key_path, int num_threads) {
    struct file *input_file, *output_file, *key_file; // Punteros a archivos de entrada, salida y clave
    loff_t input_offset = 0, output_offset = 0, key_offset = 0; // Desplazamientos de archivos
    unsigned char *key_buffer, *data_buffer; // Buffers para clave y datos
    size_t input_file_size, key_file_size; // Tamaños de archivos de entrada y clave
    struct task_parameters *task_params; // Array de parámetros de tarea
    struct task_struct **threads; // Array de punteros a hilos
    DataFragment *fragments; // Array de fragmentos de datos
    size_t fragment_size, remainder; // Tamaño de cada fragmento y resto
    int i, ret = 0;

    printk(KERN_INFO "Opening files: input=%s, output=%s, key=%s\n", input_path, output_path, key_path); // Registrar rutas de archivos

    // Abrir archivos de entrada, salida y clave
    input_file = filp_open(input_path, O_RDONLY, 0);
    output_file = filp_open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    key_file = filp_open(key_path, O_RDONLY, 0);

    // Comprobar errores al abrir archivos
    if (IS_ERR(input_file)) {
        ret = PTR_ERR(input_file);
        printk(KERN_ERR "Failed to open input file: %d\n", ret);
        goto out;
    }
    if (IS_ERR(output_file)) {
        ret = PTR_ERR(output_file);
        printk(KERN_ERR "Failed to open output file: %d\n", ret);
        goto close_input_file;
    }
    if (IS_ERR(key_file)) {
        ret = PTR_ERR(key_file);
        printk(KERN_ERR "Failed to open key file: %d\n", ret);
        goto close_output_file;
    }

    // Obtener el tamaño del archivo de clave
    key_file_size = i_size_read(file_inode(key_file));
    if (key_file_size <= 0) {
        ret = -EINVAL;
        printk(KERN_ERR "Invalid key size: %zu\n", key_file_size);
        goto close_key_file;
    }

    // Asignar memoria para el buffer de clave
    key_buffer = kmalloc(key_file_size, GFP_KERNEL);
    if (!key_buffer) {
        ret = -ENOMEM;
        printk(KERN_ERR "Failed to allocate memory for key\n");
        goto close_key_file;
    }

    // Leer el archivo de clave en el buffer de clave
    ret = kernel_read(key_file, key_buffer, key_file_size, &key_offset);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read key: %d\n", ret);
        goto free_key_buffer;
    }

    // Obtener el tamaño del archivo de entrada
    input_file_size = i_size_read(file_inode(input_file));
    if (input_file_size <= 0) {
        ret = -EINVAL;
        printk(KERN_ERR "Invalid input file size: %zu\n", input_file_size);
        goto free_key_buffer;
    }

    // Asignar memoria para el buffer de datos
    data_buffer = kmalloc(input_file_size, GFP_KERNEL);
    if (!data_buffer) {
        ret = -ENOMEM;
        printk(KERN_ERR "Failed to allocate memory for data\n");
        goto free_key_buffer;
    }

    // Leer el archivo de entrada en el buffer de datos
    ret = kernel_read(input_file, data_buffer, input_file_size, &input_offset);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read input file: %d\n", ret);
        goto free_data_buffer;
    }

    // Asignar memoria para hilos, parámetros de tarea y fragmentos
    threads = kmalloc(sizeof(struct task_struct *) * num_threads, GFP_KERNEL);
    task_params = kmalloc(sizeof(struct task_parameters) * num_threads, GFP_KERNEL);
    fragments = kmalloc(sizeof(DataFragment) * num_threads, GFP_KERNEL);

    if (!threads || !task_params || !fragments) {
        ret = -ENOMEM;
        printk(KERN_ERR "Failed to allocate memory for threads, task parameters, or fragments.\n");
        goto free_data_buffer;
    }

    // Calcular el tamaño de cada fragmento y el resto
    fragment_size = input_file_size / num_threads;
    remainder = input_file_size % num_threads;

    // Crear hilos para procesar cada fragmento
    for (i = 0; i < num_threads; i++) {
        fragments[i].data = data_buffer; // Establecer buffer de datos
        fragments[i].data_size = input_file_size; // Establecer tamaño de datos
        fragments[i].key = key_buffer; // Establecer buffer de clave
        fragments[i].key_size = key_file_size; // Establecer tamaño de clave
        fragments[i].start = i * fragment_size; // Establecer índice de inicio
        fragments[i].end = (i == num_threads - 1) ? (i + 1) * fragment_size + remainder : (i + 1) * fragment_size; // Establecer índice de fin

        printk(KERN_INFO "Creating thread %d: start=%zu, end=%zu\n", i, fragments[i].start, fragments[i].end); // Registrar creación de hilo

        task_params[i].fragment = fragments[i]; // Establecer fragmento en parámetros de tarea
        init_completion(&task_params[i].done); // Inicializar estructura de finalización

        // Crear y ejecutar el hilo
        threads[i] = kthread_run(xor_task, &task_params[i], "xor_thread_%d", i);
        if (IS_ERR(threads[i])) {
            ret = PTR_ERR(threads[i]);
            printk(KERN_ERR "Failed to create thread %d: %d\n", i, ret);
            goto free_all;
        }
    }

    // Esperar a que todos los hilos completen
    for (i = 0; i < num_threads; i++) {
        printk(KERN_INFO "Waiting for thread %d to complete\n", i); // Registrar espera de hilo
        wait_for_completion(&task_params[i].done); // Esperar a que el hilo complete
    }

    // Escribir los datos procesados en el archivo de salida
    ret = kernel_write(output_file, data_buffer, input_file_size, &output_offset);
    if (ret < 0) {
        printk(KERN_ERR "Failed to write output file: %d\n", ret);
    } else {
        printk(KERN_INFO "Output file written successfully\n"); // Registrar escritura exitosa
    }

free_all:
    kfree(threads); // Liberar memoria para hilos
    kfree(task_params); // Liberar memoria para parámetros de tarea
    kfree(fragments); // Liberar memoria para fragmentos

free_data_buffer:
    kfree(data_buffer); // Liberar memoria para buffer de datos

free_key_buffer:
    kfree(key_buffer); // Liberar memoria para buffer de clave

close_key_file:
    filp_close(key_file, NULL); // Cerrar archivo de clave

close_output_file:
    filp_close(output_file, NULL); // Cerrar archivo de salida

close_input_file:
    filp_close(input_file, NULL); // Cerrar archivo de entrada

out:
    return ret; // Devolver resultado
}

// Definición del syscall my_encrypt
SYSCALL_DEFINE4(my_encrypt, const char __user *, input_path, const char __user *, output_path, const char __user *, key_path, int, num_threads) {
    char *k_input_path, *k_output_path, *k_key_path; // Rutas en espacio de kernel
    int ret;

    printk(KERN_INFO "Entering my_encrypt syscall\n"); // Registrar entrada al syscall

    // Copiar rutas desde espacio de usuario a espacio de kernel
    k_input_path = strndup_user(input_path, PATH_MAX);
    k_output_path = strndup_user(output_path, PATH_MAX);
    k_key_path = strndup_user(key_path, PATH_MAX);

    printk(KERN_INFO "Input file path: %s\n", k_input_path); // Registrar ruta del archivo de entrada
    printk(KERN_INFO "Output file path: %s\n", k_output_path); // Registrar ruta del archivo de salida
    printk(KERN_INFO "Key file path: %s\n", k_key_path); // Registrar ruta del archivo de clave

    // Comprobar errores al copiar rutas
    if (IS_ERR(k_input_path) || IS_ERR(k_output_path) || IS_ERR(k_key_path)) {
        ret = -EFAULT;
        printk(KERN_ERR "Failed to copy paths from user space\n");
        goto out_free;
    }

    // Procesar el archivo con las rutas y el número de hilos dados
    ret = process_file(k_input_path, k_output_path, k_key_path, num_threads);

out_free:
    kfree(k_input_path); // Liberar ruta de entrada en espacio de kernel
    kfree(k_output_path); // Liberar ruta de salida en espacio de kernel
    kfree(k_key_path); // Liberar ruta de clave en espacio de kernel
    return ret; // Devolver resultado
}

// Definición del syscall my_decrypt
SYSCALL_DEFINE4(my_decrypt, const char __user *, input_path, const char __user *, output_path, const char __user *, key_path, int, num_threads) {
    char *k_input_path, *k_output_path, *k_key_path; // Rutas en espacio de kernel
    int ret;

    printk(KERN_INFO "Entering my_decrypt syscall\n"); // Registrar entrada al syscall

    // Copiar rutas desde espacio de usuario a espacio de kernel
    k_input_path = strndup_user(input_path, PATH_MAX);
    k_output_path = strndup_user(output_path, PATH_MAX);
    k_key_path = strndup_user(key_path, PATH_MAX);

    printk(KERN_INFO "Input file path: %s\n", k_input_path); // Registrar ruta del archivo de entrada
    printk(KERN_INFO "Output file path: %s\n", k_output_path); // Registrar ruta del archivo de salida
    printk(KERN_INFO "Key file path: %s\n", k_key_path); // Registrar ruta del archivo de clave

    // Comprobar errores al copiar rutas
    if (IS_ERR(k_input_path) || IS_ERR(k_output_path) || IS_ERR(k_key_path)) {
        ret = -EFAULT;
        printk(KERN_ERR "Failed to copy paths from user space\n");
        goto out_free;
    }

    // Procesar el archivo con las rutas y el número de hilos dados
    ret = process_file(k_input_path, k_output_path, k_key_path, num_threads);

out_free:
    kfree(k_input_path); // Liberar ruta de entrada en espacio de kernel
    kfree(k_output_path); // Liberar ruta de salida en espacio de kernel
    kfree(k_key_path); // Liberar ruta de clave en espacio de kernel
    return ret; // Devolver resultado
}