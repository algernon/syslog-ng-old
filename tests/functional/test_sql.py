from globals import *
from log import *
from messagegen import *
from messagecheck import *
from control import flush_files, stop_syslogng
import func_test_case

context = locals()

class TestSql(func_test_case.SyslogNGOldTest):
    config = """@version: 3.4

    options { ts_format(iso); chain_hostnames(no); keep_hostname(yes); threaded(yes); };

    source s_int { internal(); };
    source s_tcp { tcp(port(%(port_number)d)); };

    destination d_sql {
        sql(type(sqlite3) database("%(current_dir)s/test-sql.db") host(dummy) port(1234) username(dummy) password(dummy)
            table("logs")
            null("@NULL@")
            columns("date datetime", "host", "program", "pid", "msg")
            values("$DATE", "$HOST", "$PROGRAM", "${PID:-@NULL@}", "$MSG")
            indexes("date", "host", "program")
            flags(explicit-commits)
            flush-lines(25) flush_timeout(100));
    };

    log { source(s_tcp); destination(d_sql); };

    """ % context

    is_runnable = True

    def check_env(self):

        if not has_module('afsql'):
            print 'afsql module is not available, skipping SQL test'
            self.is_runnable = False
            return False
        paths=('/opt/syslog-ng/bin', '/usr/bin', '/usr/local/bin')
        found=False
        for pth in paths:
            if os.path.isfile(os.path.join(pth, 'sqlite3')):
                found = True
        if not found:
            print_user("no sqlite3 tool, skipping SQL test\nSearched: %s\n" % ':'.join(paths))
            self.is_runnable = False
            return False

        soext='.so'
        if re.match('hp-ux', sys.platform) and not re.match('ia64', os.uname()[4]):
            soext='.sl'

        found = False
        paths = (os.environ.get('dbd_dir', None), '/usr/local/lib/dbd', '/usr/lib/dbd', '/usr/lib64/dbd/', '/opt/syslog-ng/lib/dbd')
        for pth in paths:
            if pth and os.path.isfile('%s/libdbdsqlite3%s' % (pth, soext)):
                found = True
                break
        if not found:
            print_user('No sqlite3 backend for libdbi. Skipping SQL test.\nSearched: %s\n' % ':'.join(paths))
            self.is_runnable = False
            return False

        print 'sqlite3 found, proceeding to SQL tests'
        return True

    def checkRunnable(self):
        return self.check_env()

    def test_sql(self):

        if not self.check_env():
           return True
        messages = (
            'sql1',
            'sql2'
        )
        s = SocketSender(AF_INET, ('localhost', port_number), dgram=0)

        expected = []
        for msg in messages:
            expected.extend(s.sendMessages(msg, pri=7))
        print_user("Waiting for 10 seconds until syslog-ng writes all records to the SQL table")
        time.sleep(10)
        stop_syslogng()
        time.sleep(5)
        return check_sql_expected("%s/test-sql.db" % current_dir, "logs", expected, settle_time=5, syslog_prefix="Sep  7 10:43:21 bzorp prog 12345")
