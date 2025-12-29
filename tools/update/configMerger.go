package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
)

func main() {
	fmt.Println("Starting configuration file merge...")
	
	var newConfig, currentConfig string
	flag.StringVar(&newConfig, "new", "", "Path to new configuration file")
	flag.StringVar(&currentConfig, "current", "", "Path to current configuration file")


	flag.Parse()
	
	if newConfig == "" || currentConfig == "" {
		return
	}
	
	if err := mergeConfig(newConfig, currentConfig); err != nil {
		fmt.Printf("Error: %v\n", err)
		os.Exit(1)
	}
	
	fmt.Println("Configuration merge completed successfully!")
}

func mergeConfig(newPath, currentPath string) error {
	newData, err := ioutil.ReadFile(newPath)
	if err != nil {
		return fmt.Errorf("failed to read new configuration: %v", err)
	}
	
	var newConfig map[string]interface{}
	if err := json.Unmarshal(newData, &newConfig); err != nil {
		return fmt.Errorf("new configuration format error: %v", err)
	}
	
	var currentConfig map[string]interface{}
	if _, err := os.Stat(currentPath); err == nil {
		currentData, err := ioutil.ReadFile(currentPath)
		if err == nil {
			if err := json.Unmarshal(currentData, &currentConfig); err != nil {
				fmt.Printf("Warning: current configuration format error, will create new: %v\n", err)
			}
		}
	} else {
		fmt.Println("Current configuration file not found, creating new configuration.")
	}
	
	if len(currentConfig) > 0 {
		backupPath := currentPath + ".backup"
		if backupData, err := json.MarshalIndent(currentConfig, "", "  "); err == nil {
			if err := ioutil.WriteFile(backupPath, backupData, 0644); err == nil {
				fmt.Printf("Backup created: %s\n", backupPath)
			} else {
				fmt.Printf("Warning: failed to create backup: %v\n", err)
			}
		}
	}
	
	result := make(map[string]interface{})
	
	for key, value := range currentConfig {
		if _, ok := value.(map[string]interface{}); ok {
			result[key] = value
		}
	}
	
	for key, value := range newConfig {
		if _, ok := value.(map[string]interface{}); ok {
			result[key] = value
		}
	}
	
	resultData, err := json.MarshalIndent(result, "", "  ")
	if err != nil {
		return fmt.Errorf("failed to format result: %v", err)
	}
	
	if err := ioutil.WriteFile(currentPath, resultData, 0644); err != nil {
		return fmt.Errorf("failed to save configuration: %v", err)
	}
	
	fmt.Printf("Successfully merged %d configuration sections\n", len(result))
	return nil
}