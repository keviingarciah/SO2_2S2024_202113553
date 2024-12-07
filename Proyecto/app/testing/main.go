package main

import (
	"fmt"
	"math/rand"
	"net/http"
	"time"

	"github.com/rs/cors"
)

func generateRandomNumbers(ch chan<- int) {
	for {
		ch <- rand.Intn(10000000) // Genera un número aleatorio entre 0 y 9999999
		time.Sleep(1 * time.Second)
	}
}

func getMemoryData(freeCh, usedCh, cachedCh <-chan int) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		free := <-freeCh
		used := <-usedCh
		cached := <-cachedCh

		w.Header().Set("Content-Type", "text/plain")
		response := fmt.Sprintf("Free Memory: %d\nUsed Memory: %d\nCached Memory: %d", free, used, cached)
		w.Write([]byte(response))
	}
}

func getActiveInactivePages(activeCh, inactiveCh <-chan int) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		active := <-activeCh
		inactive := <-inactiveCh

		w.Header().Set("Content-Type", "text/plain")
		response := fmt.Sprintf("Active Pages: %d\nInactive Pages: %d", active, inactive)
		w.Write([]byte(response))
	}
}

func getSwapInfo(totalCh, usedCh, freeCh <-chan int) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		total := <-totalCh
		used := <-usedCh
		free := <-freeCh

		w.Header().Set("Content-Type", "text/plain")
		response := fmt.Sprintf("Total Swap: %d\nUsed Swap: %d\nFree Swap: %d", total, used, free)
		w.Write([]byte(response))
	}
}

func getPageFaults(minorCh, majorCh <-chan int) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		minor := <-minorCh
		major := <-majorCh

		w.Header().Set("Content-Type", "text/plain")
		response := fmt.Sprintf("Minor Page Faults: %d\nMajor Page Faults: %d", minor, major)
		w.Write([]byte(response))
	}
}

func getTopMemoryProcesses(memCh <-chan int) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		processes := []struct {
			PID     int
			memory  int
			Command string
		}{
			{3760, 3760, "firefox-bin"},
			{4328, 1321, "fwupd"},
			{1215, 12312, "Xorg"},
			{3836, 3242, "Privileged Cont"},
			{3994, 141412, "Isolated Web Co"},
		}

		w.Header().Set("Content-Type", "text/plain")
		response := "Top 5 Memory Consuming Processes:\n"
		for _, process := range processes {
			response += fmt.Sprintf("PID: %d, Memory: %d, Command: %s\n", process.PID, process.memory, process.Command)
		}
		w.Write([]byte(response))
	}
}

func main() {
	mux := http.NewServeMux()

	// Canales para números aleatorios con buffer
	freeCh := make(chan int, 10)
	usedCh := make(chan int, 10)
	cachedCh := make(chan int, 10)
	activeCh := make(chan int, 10)
	inactiveCh := make(chan int, 10)
	totalSwapCh := make(chan int, 10)
	usedSwapCh := make(chan int, 10)
	freeSwapCh := make(chan int, 10)
	minorFaultCh := make(chan int, 10)
	majorFaultCh := make(chan int, 10)
	memCh := make(chan int, 10)

	// Ejecutar la generación de números aleatorios en goroutines
	go generateRandomNumbers(freeCh)
	go generateRandomNumbers(usedCh)
	go generateRandomNumbers(cachedCh)
	go generateRandomNumbers(activeCh)
	go generateRandomNumbers(inactiveCh)
	go generateRandomNumbers(totalSwapCh)
	go generateRandomNumbers(usedSwapCh)
	go generateRandomNumbers(freeSwapCh)
	go generateRandomNumbers(minorFaultCh)
	go generateRandomNumbers(majorFaultCh)
	go generateRandomNumbers(memCh)

	mux.HandleFunc("/memory-usage", getMemoryData(freeCh, usedCh, cachedCh))
	mux.HandleFunc("/active-inactive-pages", getActiveInactivePages(activeCh, inactiveCh))
	mux.HandleFunc("/swap-info", getSwapInfo(totalSwapCh, usedSwapCh, freeSwapCh))
	mux.HandleFunc("/page-faults", getPageFaults(minorFaultCh, majorFaultCh))
	mux.HandleFunc("/top-memory-processes", getTopMemoryProcesses(memCh))

	// Habilitar CORS
	handler := cors.Default().Handler(mux)

	http.ListenAndServe(":8080", handler)
}
