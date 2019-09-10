class Decoder {
	buf = [];
	write = byte => {
		const array = [byte]
		var out, i, len, c;
	  var char2, char3;

	  out = "";
	  len = array.length;
	  i = 0;

	  // XXX: Invalid bytes are ignored
	  while(i < len) {
	    c = array[i++];
	    if (c >> 7 == 0) {
	      // 0xxx xxxx
	      out += String.fromCharCode(c);
	      continue;
	    }

	    // Invalid starting byte
	    if (c >> 6 == 0x02) {
	      continue;
	    }

	    // #### MULTIBYTE ####
	    // How many bytes left for thus character?
	    var extraLength = null;
	    if (c >> 5 == 0x06) {
	      extraLength = 1;
	    } else if (c >> 4 == 0x0e) {
	      extraLength = 2;
	    } else if (c >> 3 == 0x1e) {
	      extraLength = 3;
	    } else if (c >> 2 == 0x3e) {
	      extraLength = 4;
	    } else if (c >> 1 == 0x7e) {
	      extraLength = 5;
	    } else {
	      continue;
	    }

	    // Do we have enough bytes in our data?
	    if (i+extraLength > len) {
	      var leftovers = array.slice(i-1);

	      // If there is an invalid byte in the leftovers we might want to
	      // continue from there.
	      for (; i < len; i++) if (array[i] >> 6 != 0x02) break;
	      if (i != len) continue;

	      // All leftover bytes are valid.
				this.buf = leftovers
	      return out
	    }
	    // Remove the UTF-8 prefix from the char (res)
	    var mask = (1 << (8 - extraLength - 1)) - 1,
	        res = c & mask, nextChar, count;

	    for (count = 0; count < extraLength; count++) {
	      nextChar = array[i++];

	      // Is the char valid multibyte part?
	      if (nextChar >> 6 != 0x02) {break;};
	      res = (res << 6) | (nextChar & 0x3f);
	    }

	    if (count != extraLength) {
	      i--;
	      continue;
	    }

	    if (res <= 0xffff) {
	      out += String.fromCharCode(res);
	      continue;
	    }

	    res -= 0x10000;
	    var high = ((res >> 10) & 0x3ff) + 0xd800,
	        low = (res & 0x3ff) + 0xdc00;
	    out += String.fromCharCode(high, low);
	  }

		this.buf = [];
	  return out;
	}
}
window.decoder = new Decoder
