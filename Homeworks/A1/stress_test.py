import subprocess
import time
import multiprocessing

def read_users_from_file(filename):
    users = []
    with open(filename, 'r') as file:
        for line in file:
            username, password = line.strip().split(':')
            users.append((username, password))
    return users

def run_client(username, password):
    """Start a client process and keep it alive."""
    process = subprocess.Popen(
        ["./client_grp"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
    )
    process.stdin.write(f"{username}\n{password}\n")
    process.stdin.flush()  # Ensure data is sent
    print(f"User {username} connected")
    process.wait()  # Keep process running

def main():
    users = read_users_from_file("users.txt")
    processes = []

    for username, password in users:
        p = multiprocessing.Process(target=run_client, args=(username, password))
        processes.append(p)

    # Start all processes
    for p in processes:
        p.start()
        time.sleep(0.01)  # Shorter delay to stress-test better

    # Wait for all processes to complete
    for p in processes:
        p.join()

if __name__ == "__main__":
    main()
