import codecs
import base64
import hashlib
import getpass
import errno

def encrypt(file: str, passw = None) -> bool:
    """
    This function processes text by applying ROT13, converting to a Hex string, 
    and then encoding into Base64.
    Reads data from a file and saves to the same file.
    """

    try:
        # Reading the file and encrypting
        text = open(file, 'r').read()

        # Asking the user for password and then storing on the file
        if not passw:
            passw = getpass.getpass("Enter a password for encryption: ")
        passw = hashlib.sha256(passw.encode('utf-8')).hexdigest()

        if passw == text.split('\n')[0]:
        	print("[\033[0;91mERROR\033[0m] File already encrypted")
        	return False
        passw = f"{passw}\n"

        text = codecs.encode(text, 'rot_13')
        text = text.encode('utf-8').hex()
        text_final = passw + base64.b64encode(text.encode('utf-8')).decode('utf-8')

        # Saving the encrypted data back to the file
        open(file, 'w').write(text_final)
    except FileNotFoundError:
        print(f"[\033[0;91mERROR\033[0m] File not found: \033[0;97m{file}\033[0m")
        return False
    except PermissionError as e:
        if e.errno == errno.EACCES:
            print("[\033[0mPERMISSION DENIED\033[0m] Check your user read privileges")
        elif e.errno == errno.EPERM:
            print("[\033[0mNOT PERMITTED\033[0m] Higher system restrictions apply")
        else:
            print(f"[\033[0mERROR\033[0m] {e.errno} - {e.strerror}")
        return False
    except Exception as e:
        print(f"[\033[0;91mERROR\033[0m] {e}")
        return False

    return True

def decrypt(file: str, passw = None) -> bool:
    """
    This function processes text by decoding Base64, converting from a Hex string, 
    and then reversing the ROT13 shift.
    Reads data from a file and saves to the same file.
    """

    try:
        # Reading the file and decrypting
        text = open(file, 'r').read()

        # Splitting the file to get the stored hash and the encrypted data
        file_parts = text.split('\n')
        if len(file_parts) < 2:
            print("[\033[0;91mERROR\033[0m] File is not encrypted or is corrupted")
            return False
            
        stored_hash = file_parts[0]
        encrypted_data = file_parts[1]

        # Asking the user for password and then checking it
        if not passw:
            passw = getpass.getpass("Enter the password for decryption: ")
        passw = hashlib.sha256(passw.encode('utf-8')).hexdigest()

        if passw != stored_hash:
            print("[\033[0;91mERROR\033[0m] Incorrect password")
            return False

        # Reversing the encryption pipeline
        text = base64.b64decode(encrypted_data.encode('utf-8')).decode('utf-8')
        text = bytes.fromhex(text).decode('utf-8')
        text_final = codecs.decode(text, 'rot_13')

        # Saving the decrypted data back to the file
        open(file, 'w').write(text_final)
    except FileNotFoundError:
        print(f"[\033[0;91mERROR\033[0m] File not found: \033[0;97m{file}\033[0m")
        return False