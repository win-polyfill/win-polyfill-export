// https://www.geeksforgeeks.org/suffix-array-set-2-a-nlognlogn-algorithm/#

// TODO: Use radix sort to improve speed
// Do not support for big array

class SuffixArray {
  s: Uint8Array;
  n: number;
  su_body: Uint32Array;
  // su_body_u8: Uint8Array;
  su_index: Uint32Array;

  su_body_sort: BigUint64Array;
  maximalRank: number;
  maximalNext: number;

  // On little endian computer
  // struct { next rank }

  // On big endian computer
  // struct { rank next }

  constructor(s: Uint8Array) {
    this.maximalRank = 0;
    this.maximalNext = 0;
    this.s = s;
    this.n = s.length;
    this.su_body_sort = new BigUint64Array(this.n);
    this.su_body = new Uint32Array(this.su_body_sort.buffer, 0, this.n * 2);
    this.su_index = new Uint32Array(this.n);
  }

  printSu() {
    let p = "";
    for (let i = 0; i < this.n; i += 1) {
      p =
        p + this.su_index[i] + "," + this.su_body_sort[this.su_index[i]] + " ";
    }
    console.log(p);
  }

  // This is the main function that takes a string 'txt'
  // of size n as an argument, builds and return the
  // suffix array for the given string
  init_le() {
    let prev = 0;
    for (let i = this.n - 1; i >= 0; --i) {
      this.su_index[i] = i;
      const next_i = i << 1;
      const rank_i = next_i + 1;
      this.su_body[next_i] = prev;
      prev = this.s[i];
      this.su_body[rank_i] = prev;
    }

    // Sort the suffixes using the comparison function
    // defined above.
    this.su_index.sort((a, b) => {
      let va = this.su_body_sort[a];
      let vb = this.su_body_sort[b];
      return va < vb ? -1 : 1;
    });
    // console.log("new sort result:");
    // this.printSu();

    // This array is needed to get the index in suffixes[]
    // from original index. This mapping is needed to get
    // next suffix.
    for (let length = 4; length < 2 * this.n; length <<= 1) {
      // Assigning rank and index values to first suffix
      let rank = 0;
      // let prev = su[0].rank;
      // su[0].rank = rank;
      let rank_0_pos = (this.su_index[0] << 1) + 1;
      let prev = this.su_body[rank_0_pos];
      this.su_body[rank_0_pos] = rank;
      for (let i = 1; i < this.n; i++) {
        const next_i_pos = this.su_index[i] << 1;
        const rank_i_pos = next_i_pos + 1;
        const next_i_minus_1_pos = this.su_index[i - 1] << 1;
        const rank_i_fetched = this.su_body[rank_i_pos];
        // if (su[i].rank == prev && su[i].next == su[i - 1].next)
        // If first rank and next ranks are same as
        // that of previous suffix in array,
        // assign the same new rank to this suffix
        if (
          rank_i_fetched == prev &&
          this.su_body[next_i_pos] == this.su_body[next_i_minus_1_pos]
        ) {
          // prev = su[i].rank;
          this.su_body[rank_i_pos] = rank;
        } else {
          // Otherwise increment rank and assign
          // prev = su[i].rank;
          this.su_body[rank_i_pos] = ++rank;
        }
        prev = rank_i_fetched;
      }

      // Assign next rank to every suffix

      for (let i = 0; i < this.n; i++) {
        const index_i = this.su_index[i];
        let nextP = index_i + (length >> 1);
        const next_i_pos = index_i << 1;

        let valueNext = this.su_body[next_i_pos];
        let valueRank = this.su_body[next_i_pos + 1];
        if (valueNext > this.maximalNext) {
          this.maximalNext = valueNext;
        }
        if (valueRank > this.maximalRank) {
          this.maximalRank = valueRank;
        }

        if (nextP < this.n) {
          // su[i].next = su[ind[nextP]].rank;
          let rank_pos = (nextP << 1) + 1;
          let rank_value = this.su_body[rank_pos];
          this.su_body[next_i_pos] = rank_value;
        } else {
          this.su_body[next_i_pos] = 0;
        }
      }
      // Sort the suffixes according
      // to first k characters
      this.su_index.sort((a, b) => {
        let va = this.su_body_sort[a];
        let vb = this.su_body_sort[b];
        return va < vb ? -1 : 1;
      });
    }
    return this.su_index;
  }
}

function test() {
  let buf = Buffer.from("banana\x01");
  let time = Date.now();
  let bufBig = Buffer.alloc(10000000, 80);
  buf.copy(bufBig, 10);
  {
    let result = new SuffixArray(bufBig);
    console.log(result.init_le().length);
    console.log(
      `maximalRank: ${result.maximalRank} maximalNext:${result.maximalNext}`
    );
    console.log(result.su_body.length);
  }
  let timeEnd = Date.now();
  console.log("Total time consumed: ", (timeEnd - time) / 1000.0);
}
// test();

export interface SuffixArrayLoaded {
  content: Uint8Array;
  index: Uint32Array;
}

export function binary_search(sa: SuffixArrayLoaded, pattern: Uint8Array) {
  let low = 0;
  let high = sa.content.length - 1;
  while (low <= high) {
    let mid = high - ((high - low) >> 1);
    let sub_matched = sa.content.subarray(
      sa.index[mid],
      sa.index[mid] + pattern.length
    );
    let cmp_result = -1;
    if (sub_matched.length >= pattern.length) {
      cmp_result = Buffer.compare(sub_matched, pattern);
    }
    if (cmp_result == 0) {
      return mid;
    } else if (cmp_result == -1) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }
  return -1;
}

function testb() {
  let buf = Buffer.from("banana");
  {
    // 5 3 1 0 4 2
    let result = new SuffixArray(buf);
    let su_index = result.init_le();
    // result.printSu()
    console.log(result.su_index);
    let loaded: SuffixArrayLoaded = {
      content: buf,
      index: su_index,
    };
    const ret = binary_search(loaded, Buffer.from("ea"));
    console.log(ret);
    console.log(buf.subarray(ret).toString("utf-8"));
  }
}
// testb();
