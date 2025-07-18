import gdb
import gdb.printing

class Hashmap(gdb.ValuePrinter):
    def __init__(self, val) -> None:
        self.val = val

    def to_string(self):
        bucket_count = self.val['bucket_count']
        buckets = self.val['buckets']

        string = "hashmap = {\n"

        for i in range(0, bucket_count):
            bucket = buckets[i]
            while int(bucket) != 0:
                key = bucket['key']
                value = bucket['object']['value']
                size = bucket['object']['size']
                string += f'  ["{key.string()}"] = ({value}, {size})\n'
                bucket = bucket['next']

        string += "}"
        return string

def build_pretty_printers():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("noir-printers")
    pp.add_printer("hashmap", "^hashmap$", Hashmap)
    return pp 

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printers())
