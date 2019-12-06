import struct
import socket
ip_txt = '5.6.7.8'
content = 'Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer suscipit a nisl vel porta. Nullam quis nulla nec quam porttitor fringilla. Praesent ex velit, mollis quis finibus elementum, accumsan quis diam. Vivamus vulputate bibendum augue, vel faucibus velit rutrum sed. Vivamus ornare tempor orci, in aliquet leo dignissim sed. Sed et placerat ante. Duis porta, lectus sed facilisis dictum, ligula mauris eleifend turpis, et faucibus nisl orci eu mi. Nam finibus feugiat velit, sit amet egestas lorem luctus id. Suspendisse nisi augue, elementum ut turpis quis, pellentesque mollis magna. Praesent laoreet erat ac felis euismod gravida. Duis ut urna a lorem feugiat laoreet dignissim sit amet nulla. Donec non interdum libero, eu euismod ante. Nam interdum lacus vitae justo sollicitudin finibus. Phasellus eget ligula sed metus gravida tempus eget non purus. Quisque rhoncus leo massa, vitae congue nisi pharetra at. Aliquam sed ligula laoreet, ullamcorper leo rutrum, mattis nunc. Morbi cursus feugiat massa vel facilisis. Pellentesque vel orci luctus, pharetra lectus tempus, laoreet sapien. In eu venenatis libero. Maecenas elementum eu urna eget iaculis. In hendrerit lobortis lectus, et egestas diam porta ut. Nam venenatis nunc eu auctor varius. Phasellus pharetra quis eros eu tempor. Nullam a nisi lacinia, aliquam nisi eleifend, semper purus. Donec sed aliquet nibh, eget egestas erat. Pellentesque vitae odio ultricies, sodales eros eget, condimentum magna. Nam scelerisque lorem vel libero convallis placerat. Donec eget ullamcorper tortor posuere. '
content_len = len(content)
with open('test.bin', 'wb') as outF:
    outF.write(socket.inet_aton(ip_txt))
    outF.write(struct.pack('i', content_len))
    outF.write(str.encode(content))