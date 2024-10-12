import requests
import sys
import os

def read_file(file_path):
    """Read the integers from a file."""
    with open(file_path, 'r') as file:
        content = file.read().strip().replace('\n',',')
        numbers = list(map(int, content.split(',')))
    return numbers

def create_payload(numbers, value, instance):
    """Create the JSON payload."""
    payload = {
        "sender": {"id": "20220060569"},
        "solution": {
            "anwser": numbers,
            "value": value,
            "instance": instance
        }
    }
    return payload

def send_post_request(url, payload):
    """Send a POST request with the given payload."""
    headers = {"Content-Type": "application/json"}
    response = requests.post(url, headers=headers, json=payload)
    return response

def main(file_path):
    try:
        numbers = read_file(file_path)
        
        # Assuming the last number in the list is the value
        value = numbers.pop()  # Remove the last element and use it as the value
        
        # Get the instance name from the file name without extension
        instance = os.path.splitext(os.path.basename(file_path))[0]
        
        payload = create_payload(numbers, value, instance)
        
        url = "https://copa-apa.vercel.app/api/submit"
        response = send_post_request(url, payload)
        
        print(f"Response Status Code: {response.status_code}")
        print(f"Response Body: {response.json()}")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <file_path>")
        sys.exit(1)

    file_path = sys.argv[1]
    main(file_path)
