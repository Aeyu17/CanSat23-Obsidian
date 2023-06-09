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

// func ConvertTxtToCSV(txtFile string, csvFile string) {
// 	csv, err := os.OpenFile(csvFile, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0644)
// 	defer func () {
// 		csvFile.Close()
// 	}()

// 	if err != nil {
// 		log.Println(err)
// 		return
// 	}
// 	csvFile.close()

// 	txt, err := os.OpenFile(txtFile, os.O_RDONLY, 0644)
// 	defer func () {
// 		txtFile.Close()
// 	}()

// 	if err != nil {
// 		log.Println(err)
// 		return
// 	}

// 	r := txt.NewReader(txtFile)
// 	// e := csv.NewReader(csvFile)

// 	var txtLine []string
// 	lineIgnore := false 

// 	for i := 0; i < lineIndex + 1; i++ {
// 		txtLine, err = r.Read()

// 		if err == io.EOF {
// 			lineIndex = -1
// 			break
// 		}
// 		if err != nil {
// 			log.Println(err)
// 			return
// 		}

// 	}
// 	lineIndex++

// 	csv, err := os.OpenFile(csvFile, os.O_WRONLY|os.O_CREATE|os.O_APPEND, 0644)
// 	defer func () {
// 		csvFile.Close()
// 	}()

// 	if err != nil {
// 		log.Println(err)
// 		return
// 	}

// 	for index, str := range cmdArr {
// 		pcmd += str
// 		if index != len(cmdArr) - 1 {
// 			pcmd += ","
// 		}
// 	}
// 	return 
// }

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

