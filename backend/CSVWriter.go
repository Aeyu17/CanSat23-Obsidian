package backend

import (
	"log"
	"os"
)

//CSV Writer used to output data into a csv file format
func WriteToCSV(message string) {
	//will create a new file if one does not already exist
	newFileC, err := os.OpenFile("flightdata.csv", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	defer newFileC.Close()

	//logs error if file could not open
	if err != nil {
		log.Fatal(err)
	}

	_, err2 := newFileC.WriteString(message)

	//logs error if file could not be wrote to
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
