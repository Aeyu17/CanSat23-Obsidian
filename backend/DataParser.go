package backend

import (
	"fmt"
)

func ParseData(dataSet string) (dataArray[20] string) {
	j := 0
	for i := 0; i < 20; i++ {
		var f string
		for j != len(dataSet) && string(dataSet[j]) != "," {
			f = f + string(dataSet[j])
			j++
		}
		dataArray[i] = f
		j+=2
	}
	for i := 0; i < 20; i++ {
		fmt.Println(dataArray[i])
	}
	return
}