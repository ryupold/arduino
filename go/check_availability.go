package main

import (
	"fmt"
	"io/ioutil"
	"net/http"
)

func main() {
	for {
		resp, _ := http.Get("http://192.168.1.129")
		if resp != nil {
			b, _ := ioutil.ReadAll(resp.Body)
			fmt.Println(string(b))
		}
	}
}
