from SocketDriver import SimpleSocket


class Metrics:
    def __init__(self, ip, port):
        self.metrics = {}
        self.ip = ip
        self.port = port
        self.driver = SimpleSocket(self.ip, self.port)

    def _get_efu_command(self, cmd):
        res = self.driver.Ask(cmd)

        if res.find("Error") != -1:
            print("Error getting EFU command")
            sys.exit(1)
        return res

    def get_number_of_stats(self):
        return int(self._get_efu_command('STAT_GET_COUNT').split()[1])

    def get_all_metrics(self, num_metrics):
        for i in range(1, num_metrics + 1):
            res =  self._get_efu_command('STAT_GET ' + str(i)).split()
            name = res[1]
            value = int(res[2])
            self.metrics[name] = value

    def return_metric(self, name):
        try:
            return self.metrics[name]
        except:
            return -1

    def compare_metric(self, name, value):
        try:
            return self.metrics[name] == value
        except:
            return False
