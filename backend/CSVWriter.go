package backend
import (
	"log"
	"os"
)

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
