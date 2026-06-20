<#
------- * -------
RESIGN SCRIPT
------- * -------

System: Windows OS 8.1-10
Platform: Powershell 5.1 (supported for latest versions too)

This script automates the tasks when I resign from a particular desktop system.
- Deletes all github configuration on the user (--global)
- Deletes all scoop apps if installed
- Deletes all firefox browsing

Requires admin/root access
------- * -------
#>

# Declaring the arguments for the script
param (
	[switch]$Safe,
	[switch]$Execute,
	[switch]$Help
)

# Declaring the required functions
# - - -
function deleteGitConfig {
	# This function deletes all the configuration for the GIT user / system wide

	if (Get-Command git -ErrorAction SilentlyContinue) {
		# If GIT is installed

		Write-Host "`n[~] Git found. Deleting all the configuration..." -ForegroundColor Yellow
		if (Test-Path "$HOME/.gitconfig") {
			# If the .gitconfig directory exists at HOME

			# Removing the git global configuration (if not SAFE mode)
			if ($args[0] -eq "execute"){
				git config --global --remove-section user 2>$null
				git config --global --remove-section credential 2>$null

				Remove-Item "$HOME\.gitconfig" -Force -ErrorAction SilentlyContinue
			}
			Write-Host "[~] Global .gitconfig removed" -ForegroundColor Green
		}

		# Removing system wide configuration if exists (if not SAFE mode)
		if ($args[0] -eq "execute"){
			git config --system --remove-section user 2>$null
			git config --system --remove-section credential 2>$null
		}

		# Removing the GitHub credentials from Windows Credential Manager if exists
		Write-Host "[~] Purging stored GitHub tokens from Windows Credential Manager..." -Foreground Yellow
		# - - - 
		# Finding all github.com matches
		$creds = cmdkey /list | Select-String "Target:.*github\.com"
		foreach ($cred in $creds) {
			# Extract the target name from the cmdkey output

			if ($cred -match "Target:\s*(.*)") {
				$target = $matches[1].Trim()
				Write-Host "[~] Removing credential: $target" -ForegroundColor Yellow
				if ($args[0] -eq "execute") { cmdkey /delete:$target | Out-Null }
				}
			}

			Write-Host "[*] GitHub configuration and credentials have been unset." -ForegroundColor Green
		} else {
			Write-Host "`n[!] Git is not installed" -ForegroundColor Yellow
		}
	}

	function deleteScoop {
		# This function deletes all the scoop applications which are installed via scoop

		if (Test-Path $HOME/scoop/apps) {
			$apps = Get-ChildItem -Path $HOME/scoop/apps -Directory | Where-Object { $_.Name -ne 'scoop' } | Select-Object -ExpandProperty Name

			if ($apps) {
				Write-Host "`n[~] Found installed Scoop apps. Starting uninstallation..." -ForegroundColor Yellow
				
				# Looping through each discovered app folder and uninstall it (only if not SAFE mode)
				foreach ($app in $apps) {
					Write-Host "[~] Uninstalling: $app" -ForegroundColor Yellow
					if ($args[0] -eq "execute" ) { scoop uninstall $app }
					}
					
					Write-Host "[*] All Scoop apps have been uninstalled." -ForegroundColor Green
				} else {
					Write-Host "[!] No third-party Scoop apps found in $scoopAppsPath." -ForegroundColor Yellow
				}
			} else {
				Write-Host "`n[!] Scoop directory not found at $scoopAppsPath. Is Scoop installed?" -ForegroundColor Red
			}
		}

		function deleteFirefoxData {
			# This functions deletes the firefox data and uninstalls the application (requires sudo access)

			# Firefox paths are stored in both Roaming (History, Cookies) and Local (Cache)
			$roamingpath = "$HOME\AppData\Roaming\Mozilla\Firefox"
			$localpath   = "$HOME\AppData\Local\Mozilla\Firefox"

			Write-Host "`n[~] Scanning for Firefox data paths..." -ForegroundColor Yellow

			if ((Test-Path $roamingpath) -or (Test-Path $localpath)) {
				
				if ($args[0] -eq "execute") {
					Write-Host "[~] Force-closing any running Firefox instances..." -ForegroundColor Yellow

					# Forcing to close the browser to release file locks on the profile databases
					Get-Process -name "firefox" -ErrorAction SilentlyContinue | Stop-Process -Force
					Start-Sleep -Seconds 1 # Give the system a brief moment to clear the processes
					
					# Clearing roaming profiles (History, Logins, Cookies, Bookmarks)
					if (Test-Path $roamingpath) {
						Write-Host "[*] Purging Firefox Roaming data (History, Session, Profiles)..." -ForegroundColor Green
						Remove-Item -Path "$roamingpath\*" -Recurse -Force -ErrorAction SilentlyContinue
					}

					# Clearing local profiles (Web cache, image cache, temporary assets)
					if (Test-Path $localpath) {
						Write-Host "[*] Purging Firefox Local data (Cache, Thumbnails)..." -ForegroundColor Green
						Remove-Item -Path "$localpath\*" -Recurse -Force -ErrorAction SilentlyContinue
					}

					Write-Host "[*] Firefox browsing tracks have been completely wiped." -ForegroundColor Green
				} 
			else {
				# Simulatig the SAFE mode process 

				Write-Host "[~] [SAFE MODE] Would force-close 'firefox.exe' processes if running." -ForegroundColor Gray
				if (Test-Path $roamingpath) {
					Write-Host "[~] [SAFE MODE] Would recursively delete Roaming data at: $roamingpath" -ForegroundColor Gray
				}
				if (Test-Path $localpath) {
					Write-Host "[~] [SAFE MODE] Would recursively delete Cache data at: $localpath" -ForegroundColor Gray
				}
			}
		} 
	else {
		Write-Host "[!] No Firefox user profile data detected on this profile." -ForegroundColor Yellow
	}
}
# - - -

# Checking for arguments
if ($Help) {
	# Printing the help text

	Write-Host "`nUsage: ./script.ps1 [-Safe] [-Execute] [-Help]" -ForegroundColor Cyan
	Write-Host "`n-Help`t`tDisplays this menu"
	Write-Host "-Execute`tExecutes the script in normal mode (Do not run in test environment)"
	Write-Host "-Safe`t`tExecutes the script in safe mode (For test environment)`n"
}
elseif ($Safe -and $Execute) {
	# Preventing the user from passing both active modes at the same time
	
	Write-Error "[!] Invalid usage: You cannot specify both -Safe and -Execute at the same time." -Foreground Red
}
elseif ($Safe) {
	# Handling the safe mode

	Write-Host "`n[ ------- Executing in SAFE mode ------- ]" -Foreground Cyan
	deleteGitConfig "safe"
	deleteScoop "safe"
	deleteFirefoxData "safe"
}
elseif ($Execute) {
	# Handling the execute mode
	
	deleteGitConfig "execute"
	deleteScoop "execute"
	deleteFirefoxData "execute"
}
else {
	# Handling the error arguments
	
	Write-Error "[!] Missing arguments: You must supply either -Safe, -Execute, or -Help."
}