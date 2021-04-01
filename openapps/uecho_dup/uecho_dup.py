import socket
import time
from random import randint

import colorama as c

import concurrent.futures

c.init()


def udp_send_and_receive(hisPort, hisAddress, myPort, myAddress, succ, fail, delays):

    #colors
    if(hisPort == 7):
        color = c.Back.GREEN
    else:
        color = c.Back.BLUE

    #log
    output = []
    output += [color + c.Fore.BLACK + 'ECHO REQUEST {0} at Port {1}'.format(i,hisPort) + c.Style.RESET_ALL]
    output += ['  - Address    [{0}]:{1} --> [{2}]:{3}'.format(myAddress, myPort, hisAddress, hisPort)]
    output += ['  - Payload    {0} ({1} bytes)'.format(pretty_print(request), len(request))]
    output = '\n'.join(output)
    print output

    # open socket
    socket_handler = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    socket_handler.settimeout(timeout)
    socket_handler.bind((myAddress, myPort))

    # send request
    socket_handler.sendto(request, (hisAddress, hisPort))
    startTime = time.time()

    # wait for reply
    try:
        reply, dist_addr = socket_handler.recvfrom(2048)
    except socket.timeout:
        # account
        fail += 1

        # log
        print '\n'
        print c.Back.YELLOW + c.Fore.BLACK + "TIMEOUT" + c.Style.RESET_ALL
        print "\n=========================================\n"

    else:
        if reply == request:
            succ += 1
            delay = time.time() - startTime
            delays += [delay]

            # log
            output = ['\n']
            output += [color + c.Fore.BLACK + 'ECHO RESPONSE {0} at Port {1}'.format(i, hisPort) + c.Style.RESET_ALL]
            output += ['  - Address    [{0}]:{1}->[{2}]:{3}'.format(dist_addr[0], dist_addr[1], myAddress, myPort)]
            output += ['  - Payload    {0} ({1} bytes)'.format(pretty_print(reply), len(reply))]
            output += ['  - Delay      {0:.03f}s'.format(delay)]
            output = '\n'.join(output)

        else:
            fail += 1

            output = ['\n']
            output += [c.Back.RED + c.Fore.BLACK + 'ECHO FAILED {0}'.format(i) + c.Style.RESET_ALL]
            output += ['  - Response does not match request..']
            output += ['  - Payload    {0} ({1} bytes)'.format(pretty_print(reply), len(reply))]
            output = '\n'.join(output)

        print output

        print "\n=========================================\n"

    # close socket
    socket_handler.close()
    return succ,fail,delays

def print_statistics(succ, fail, delays, hisPort):
    output = []
    output += ['\nStatistics Port: ' + str(hisPort)]
    output += ['  - success            {0}'.format(succ)]
    output += ['  - fail               {0}'.format(fail)]

    if len(delays) > 0:
        output += ['  - min/max/avg delay  {0:.03f}/{1:.03f}/{2:.03f}'.format(
            min(delays),
            max(delays),
            float(sum(delays)) / float(len(delays)))]

    output = '\n'.join(output)
    print output


def pretty_print(lst):
    lst = list(bytearray(lst))
    pld_str = ''

    pld_str = ''.join([pld_str, '[ '])
    if len(lst) <= 10:
        pld_str = ''.join([pld_str, "".join('0x{:02x} '.format(x) for x in lst)])
    else:
        pld_str = ''.join([pld_str, ''.join('0x{:02x} '.format(lst[i]) for i in range(10))])
        pld_str = ''.join([pld_str, '... '])
    pld_str = ''.join([pld_str, ']'])
    return pld_str


print "\n  UDP Echo Application to Port 7 and 8"
print "  ------------------------------------\n"
print "sends requests to both ports concurrently\n\n"

num_tries = raw_input("> Number of echoes [5]? ")
pld_size = raw_input("> Payload size [50]? ")
address = raw_input("> Destination address [bbbb::1415:92cc:0:3] (only last byte)? ")
timeout = raw_input("> Set timeout value [5]? ")
#hisPort = raw_input("> which port? [7]? ")

print '\n Starting ...\n'

if num_tries == '':
    num_tries = 5
else:
    num_tries = int(num_tries)

if pld_size == '':
    pld_size = 50
else:
    pld_size = int(pld_size)

if address == '':
    hisAddress = 'bbbb:0:0:0:1415:92cc:0:3'
else:
    hisAddress = 'bbbb:0:0:0:1415:92cc:0:' + address

if timeout == '':
    timeout = 5
else:
    timeout = int(timeout)


succ1 = 0
fail1 = 0
myAddress = ''  # means 'all'
myPort1 = randint(1024, 65535)
myPort2 = randint(1024, 65535)
delays1 = []

succ2 = 0
fail2 = 0
delays2 = []

hisPort1 = 7
hisPort2 = 8

for i in range(num_tries):

    # generate a new random payload for each echo request
    request = "".join([chr(randint(0, 255)) for j in range(pld_size)])

    with concurrent.futures.ThreadPoolExecutor() as executor:
        future1 = executor.submit(udp_send_and_receive, hisPort1, hisAddress, myPort1, myAddress, succ1, fail1, delays1)
        future2 = executor.submit(udp_send_and_receive, hisPort2, hisAddress, myPort2, myAddress, succ2, fail2, delays2)
        succ1,fail1,delays1 = future1.result()
        succ2,fail2,delays2 = future2.result()
        #succ1,fail1,delays1 = udp_send_and_receive(hisPort1, hisAddress, myPort, myAddress, succ1, fail1, delays1)
        #succ2,fail2,delays2 = udp_send_and_receive(hisPort2, hisAddress, myPort, myAddress, succ2, fail2, delays2)
        

print_statistics(succ1, fail1, delays1, hisPort1)
print_statistics(succ2, fail2, delays2, hisPort2)

    

raw_input("\nPress return to close this window...")
