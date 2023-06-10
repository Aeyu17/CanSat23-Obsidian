package backend

import (
	"encoding/csv"
	"log"
	"os"
	"io"
	"strings"
)

var lineIndex int = 0

//CSV Writer used to output data into a csv file format
func WriteToCSV(message string, csv string) {
	newFileC, err := os.OpenFile(csv, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)

	if err != nil {
		log.Println(err)
		return
	}

	defer newFileC.Close()

	if message[len(message)-1:] != "\n" {
		message = message + string("\n")
	}

	_, err2 := newFileC.WriteString(message)

	//logs error if file could be wrote to
	if err2 != nil {
		log.Println(err2)
		return
	}
}

func ReadPressureCSV(pcsv string) (pcmd string){
	csvfile, err := os.OpenFile(pcsv, os.O_RDONLY, 0644)
	defer func () {
		csvfile.Close()
	}()

	if err != nil {
		log.Println(err)
		return
	}

	r := csv.NewReader(csvfile)

	var cmdArr []string

	for i := 0; i < lineIndex + 1; i++ {
		cmdArr, err = r.Read()

		if err == io.EOF {
			lineIndex = -1
			break
		}
		if err != nil {
			log.Println(err)
			return
		}

	}
	lineIndex++

	for index, str := range cmdArr {
		pcmd += str
		if index != len(cmdArr) - 1 {
			pcmd += ","
		}
	}
	return 
}

func ReadPressureTXT(ptxt string) (pcmd string){
	txtfile, err := os.OpenFile(ptxt, os.O_RDONLY, 0644)
	defer func () {
		txtfile.Close()
	}()

	if err != nil {
		log.Println(err)
		return
	}

	r := bufio.NewReaderSize(txtfile, 1024)

	var readLine string

	for i := 0; i < lineIndex; i++ {
		readLine, err = r.ReadString('\n')
	}

	cmd := strings.Split(readLine, ",")
	for (readLine[0] == "#") {
		readLine, err = r.ReadString('\n')
		cmd = strings.Split(readLine, ",")
	}
	
	if err != nil {
		log.Println(err)
		return
	}

	for i := 0; i < len(cmd); i++ {
		if (strings.Contains(cmd[i], "$")) {
			cmd[i] = "1070"
		}
	}

	lineIndex++

	for index, str := range cmd {
		pcmd += str
		if index != len(cmd) - 1 {
			pcmd += ","
		}
	}
	txtfile.Close()
	return 
}