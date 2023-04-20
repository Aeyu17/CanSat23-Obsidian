package backend

import (
	"encoding/csv"
	"log"
	"os"
	"io"
)

var lineIndex int = 0

//CSV Writer used to output data into a csv file format
func WriteToCSV(message string, csv string) {
	newFileC, err := os.OpenFile(csv, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	defer func () {
		newFileC.Close()
	}()

	if err != nil {
		log.Fatal(err)
	}

	if message[len(message)-1:] != "\n" {
		message = message + string("\n")
	}

	_, err2 := newFileC.WriteString(message)

	//logs error if file could be wrote to
	if err2 != nil {
		log.Fatal(err2)
	}
}


func ReadPressureCSV(pcsv string) (pcmd string){
	csvfile, err := os.OpenFile(pcsv, os.O_RDONLY, 0644)
	defer func () {
		csvfile.Close()
	}()

	if err != nil {
		log.Fatal(err)
	}

	r := csv.NewReader(csvfile)

	var cmdArr []string

	for i := 0; i < lineIndex + 1; i++ {
		cmdArr, err = r.Read()

		if err == io.EOF {
			lineIndex = 0
			break
		}
		if err != nil {
			log.Fatal(err)
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

