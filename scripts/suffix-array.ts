// https://www.geeksforgeeks.org/suffix-array-set-2-a-nlognlogn-algorithm/#

import { off } from "process";

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

export interface SuffixArrayRange {
  low: number;
  high: number;
}

export function binary_search_range(
  sa: SuffixArrayLoaded,
  char: number,
  offset: number,
  range: SuffixArrayRange
): SuffixArrayRange {
  let low = range.low;
  let high = range.high;
  let rangeResult = {
    high: high,
    low: low,
  };
  while (low <= high) {
    let mid = high - ((high - low) >> 1);
    let index_mid = sa.index[mid] + offset;
    let char_mid = sa.content[index_mid] ?? 0;
    if (char > char_mid) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }
  rangeResult.low = low;
  let delta = 1;
  let new_high = low;
  for (; new_high <= range.high; ) {
    let index_new_high = sa.index[new_high] + offset;
    let char_new_high = sa.content[index_new_high] ?? 0;
    if (char_new_high == char) {
      low = new_high;
      new_high += delta;
      delta <<= 1;
    } else {
      high = new_high;
      break;
    }
  }
  high = Math.min(new_high, range.high);
  while (low <= high) {
    let mid = high - ((high - low) >> 1);
    let index_mid = sa.index[mid] + offset;
    let char_mid = sa.content[index_mid] ?? 0;
    if (char < char_mid) {
      high = mid - 1;
    } else {
      low = mid + 1;
    }
  }
  rangeResult.high = high;
  return rangeResult;
}

export function binary_search(sa: SuffixArrayLoaded, pattern: Uint8Array):SuffixArrayRange {
  let result = {
    low: 0,
    high: sa.content.length - 1,
  }
  for (let i = 0; i < pattern.length; i +=1)
  {
    result = binary_search_range(sa, pattern[i], i, result)
  }
  return result;
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
    const ret = binary_search(loaded, Buffer.from("an"));
    console.log(ret);
    console.log(buf.subarray(ret.low).toString("utf-8"));
    let range0 = binary_search_range(loaded, "a".charCodeAt(0), 0, {
      low: 0,
      high: buf.length - 1,
    });
    console.log(range0);
    let range1 = binary_search_range(loaded, "e".charCodeAt(0), 0, {
      low: 0,
      high: buf.length - 1,
    });
    console.log(range1);
    let range2 = binary_search_range(loaded, "\x01".charCodeAt(0), 0, {
      low: 0,
      high: buf.length - 1,
    });
    console.log(range2);
    let range3 = binary_search_range(loaded, "\xff".charCodeAt(0), 0, {
      low: 0,
      high: buf.length - 1,
    });
    console.log(range3);
    let range4 = binary_search_range(loaded, "n".charCodeAt(0), 0, {
      low: 0,
      high: buf.length - 1,
    });
    console.log(range4);
  }
}
// testb();
