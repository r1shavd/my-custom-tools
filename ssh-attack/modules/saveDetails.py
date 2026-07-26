import json

logs = []

def saveLogFile(host, passw):
	global logs
	logs.append({
		"host": host,
		"user": "root",
		"passw": passw
		})
	with open("tests/logs.json", "w") as logfile:
		json.dump(logs, logfile)
		logfile.close()