# THIS TEST FILE IS USING THE "www_root" FILE AS IT'S <SERVER ROOT>
import socket
from threading import Thread
import time
import glob
import os

port = 0
workers_used = 1


def good_message_connections(good_message, good_message_id):
    host = 'localhost'
    s_test = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s_test.connect((host, port))
    s_test.send(bytes(good_message, "utf-8"))
    print(f"Thread GOOD_MSG{good_message_id} sent: {good_message}\n")
    s_test.send(b"\r\n\r\n")
    data = s_test.recv(10000)
    print(f"Thread GOOD_MSG{good_message_id} recieved: {data}")
    s_test.close()


def bad_message_connections(bad_message, bad_message_id):
    host = 'localhost'
    s_test = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s_test.connect((host, port))
    s_test.send(bytes(bad_message, "utf-8"))
    print(f"Thread BAD_MSG{bad_message_id} sent: {bad_message}\n")
    s_test.send(b"\r\n\r\n")
    data = s_test.recv(10000)
    print(f"Thread GOOD_MSG{bad_message_id} recieved: {data}\n")
    s_test.close()


def slow_message_connection(broken_message):
    host = 'localhost'
    s_test = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s_test.connect((host, port))
    for char in broken_message:
        s_test.send(bytes(char, "utf-8"))
        print(f"Thread SLOW_MSG0 sent: {char}\n")
        time.sleep(1)
    s_test.send(b"\r\n\r\n")
    data = s_test.recv(10000)
    print(f"Thread SLOW_MSG0 recieved: {data}\n")
    s_test.close()


def mass_worker_connection(worker_id):
    host = 'localhost'
    s_test = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s_test.connect((host, port))
    s_test.send(b"GET / /1.1\nHost: localhost\n\n")
    s_test.send(b"\r\n\r\n")
    print(f"MASS WORKER{worker_id} sending: "
          "GET / /1.1\\nHost: localhost\\n\\n\n")
    data = s_test.recv(10000)
    print(f"MASS WORKER{worker_id} recieving: {data}\n")
    s_test.close()


def mass_worker_gen(worker_list):
    for i in range(100):
        worker_list.append(Thread(target=mass_worker_connection, args=(i,)))
    return worker_list


def worker_start(message_list):
    worker_list = []
    for message in message_list:
        worker_list.append(Thread(target=good_message_connections,
                                  args=(message,
                                        message_list.index(message))))

    return worker_list


def bad_messages(worker_list):
    bad_message_list = []
    http_one = "GET "
    http_two = " /1.1\nHost: localhost\n\n"
    bad_message_list.append(f"{http_one}/random_file.html{http_two}")
    bad_message_list.append(f"{http_one}/cgi-bin/random_script{http_two}")
    bad_message_list.append(f"{http_one}/cant_be_real.html{http_two}")
    bad_message_list.append(f"{http_one}/cgi-bin/never_seen_this{http_two}")
    bad_message_list.append(f"{http_one}/garbage_file.html{http_two}")
    for message in bad_message_list:
        worker_list.append(Thread(target=bad_message_connections,
                                  args=(message,
                                        bad_message_list.index(message))))
    return worker_list


def good_messages(root_dir):
    msg_list = list()
    cgi_flag = False
    error_flag = False
    http_one = "GET "
    http_two = " /1.1\nHost: localhost\n\n"
    for files in glob.glob(root_dir):
        tmp_lst = files.split('/')
        for i in tmp_lst:
            if i == "error":
                error_flag = True
            if i == "cgi-bin":
                tmp_lst[-1] = f"/cgi-bin/{tmp_lst[-1]}"
                cgi_flag = True
                break
        if error_flag:
            error_flag = False
            continue
        if cgi_flag:
            cgi_flag = False
        else:
            tmp_lst[-1] = f"/{tmp_lst[-1]}"
        msg_list.append(f"{http_one}{tmp_lst[-1]}{http_two}")

    msg_list.append(f"{http_one}/{http_two}")
    return msg_list


def main():
    global port
    if(os.getuid() <= 1024):
        port = os.getuid() + 2000
    else:
        port = os.getuid()

    global workers_used
    root_dir = "www_root"
    good_message_list = good_messages(f"{root_dir}/*/*")
    worker_list = worker_start(good_message_list)
    slow_thread = (Thread(target=slow_message_connection,
                          args=(good_message_list[0],)))
    worker_list = mass_worker_gen(worker_list)
    worker_list = bad_messages(worker_list)

    start = time.time()
    slow_thread.start()
    for worker in worker_list:
        worker.start()
        # uncomment this line inorder to have a slower
        # speed and a output with more readability
        # without the comment code takes ~38 seconds
        # with comment ~ 38 seconds
        # time report is wrong because of threading without the comment
        time.sleep(1)
        workers_used += 1
    slow_thread.join()
    print(f"Amount of workers used: {workers_used}")
    print(f"Time taken to run: {time.time() - start}")


if __name__ == "__main__":
    main()
