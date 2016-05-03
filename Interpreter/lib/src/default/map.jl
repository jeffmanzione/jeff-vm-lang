
class Map {
    field table, size
    def new(size) {
        self.table = []
        self.size = size
        self.table:(size-1) <- []
    }
    
    def map_hash(key) hash_uint32_t_(key) % self.size
    
    def get(key) {
        hv = self.map_hash(key)
        if ~self.table[hv] return False

        for i=0, i < |self.table[hv]|, i=i+1
            if key == self.table[hv][i][0]
                return self.table[hv][i][1]
            
        return False
    }
    
    def put(key, value) {
        hv = self.map_hash(key)
        if self.table[hv] {
            for i=0, i < |self.table[hv]|, i=i+1
                if key == self.table[hv][i][0] {
                    self.table[hv][i][1] = value
                    return
                }
            self.table[hv]:0 <- (key, value)
        } else
            self.table[hv] = [(key, value)]
    }
}