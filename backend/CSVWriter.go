package backend
import (
	"log"
	"os"
)

//CSV Writer used to output data into a csv file format
func WriteToCSV(message string) {
	newFileC, err := os.OpenFile("flightdata.csv", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	defer newFileC.Close()

	//logs error if file could not open
	if err != nil {
		log.Fatal(err)
	}

	_, err2 := newFileC.WriteString(message)

	//logs error if file could be wrote to
	if err2 != nil {
		log.Fatal(err2)
	}
}

//.TXT Writer used to output data into a .txt file format
func WriteToTXT(message string) {
    newFileT, err := os.OpenFile("testflightdata.txt", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
    defer newFileT.Close()

    if err != nil {
        log.Fatal(err)
    }

    _, err2 := newFileT.WriteString(message)

    if err2 != nil {
        log.Fatal(err2)
    }
}

//example for use
// func main(){
// 	i := 1
//     for i <= 3 {
//         WriteToCSV("epicness\n")
//         i = i + 1
//     }
// }
