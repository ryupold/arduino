package main

import (
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"strconv"
	"strings"
	"time"
)

func main() {
	ip := os.Args[1]
	fmt.Printf("connecting to: %s\n", ip)
	client := &http.Client{
		Timeout: 500 * time.Millisecond,
	}

	sum := 0
	sampleCount := 10

	for {
		resp, err := client.Get(fmt.Sprintf("http://%s", ip))

		if err != nil {
			// fmt.Println(err)
		} else {
			b, _ := ioutil.ReadAll(resp.Body)
			response := strings.TrimSpace(string(b))
			value, err := strconv.Atoi(response)
			if err == nil {
				fmt.Printf("%s: %v\n", time.Now(), value)
				sampleCount++
				sum += value
			}
		}

		time.Sleep(5000 * time.Millisecond)
	}
}
