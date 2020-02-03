
from requests import Session

url = 'http://localhost:8001'
s = Session()
task = s.post(url + '/sum', headers={'Type': 'worker'})
while True:
    response = {'result': task['a'] + task['b']}
    task = s.post(url, json=response)
