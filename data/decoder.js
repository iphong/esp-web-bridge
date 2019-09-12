class StringDecoder {
	buf = []

	constructor(encoding = 'utf8') {
		this.encoding = encoding
	}

	available() {
		return this.buf.length
	}

	read() {
		switch (this.encoding) {
			case 'bin':
				return this.encode_int(2, 8)
			case 'hex':
				return this.encode_int(16, 2)
			case 'dec':
				return this.encode_int(10, 3)
			case 'utf8':
				return this.encode_utf8()
			case 'ascii':
				return this.encode_ascii()
		}
	}

	write = data => {
		if (typeof data === 'string')
			this.decode_utf8(data)
		else if (data instanceof ArrayBuffer) {
			const view = new DataView(data)
			while (view.byteOffset < view.byteLength)
				this.buf.push(view.getUint8(view.byteOffset++))
		}
	}

	encode_int(radix, width) {
		return this.buf.splice(0, this.buf.length).map(c => {
			let value = c.toString(radix)
			while (value.length < width) value = '0' + value
			return value + ' '
		}).join('')
	}

	encode_ascii() {
		return this.buf.splice(0, this.buf.length).map(c => String.fromCodePoint(c)).join('')
	}

	decode_utf8 = (str) => {
		let i = 0
		while (i < str.length) {
			let char = str.codePointAt(i++)
			if (char >> 13) {
				this.buf.push((char >> 18 & 0b111) | (0b1111 << 4)) // 4
				this.buf.push((char >> 12 & 0b1111) | (0b111 << 5)) // 3
				this.buf.push((char >> 6 & 0b111111) | (0b1 << 7)) // 2
				this.buf.push((char >> 0 & 0b111111) | (0b1 << 7)) // 1
				continue
			}
			if (char >> 6) {
				this.buf.push((char >> 12 & 0b1111) | (0b111 << 5)) // 3
				this.buf.push((char >> 6 & 0b111111) | (0b1 << 7)) // 2
				this.buf.push((char >> 0 & 0b111111) | (0b1 << 7)) // 1
				continue
			}
			if (char & (1 << 7)) {
				this.buf.push((char >> 6 & 0b111111) | (0b1 << 7)) // 2
				this.buf.push((char >> 0 & 0b111111) | (0b1 << 7)) // 1
				continue
			}
			this.buf.push(char)
		}
	}
	encode_utf8 = () => {
		let char, char2, char3, char4, output = []

		while (char = this.buf.shift()) {
			switch (char >> 4) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
					// 0xxxxxxx
					output.push(char)
					break
				case 12:
				case 13:
					// 110x xxxx   10xx xxxx
					char2 = this.buf.shift()
					output.push(((char & 0x1F) << 6) | (char2 & 0x3F))
					break
				case 14:
					// 1110 xxxx  10xx xxxx  10xx xxxx
					[char2, char3] = this.buf.slice(0, 2)
					output.push(((char & 0x0F) << 12) | ((char2 & 0x3F) << 6) | ((char3 & 0x3F) << 0))
					break
				case 15:
					// 1111 0xxx 10xx xxxx 10xx xxxx 10xx xxxx
					[char2, char3, char4] = this.buf.slice(0, 3)
					output.push(((char & 0x07) << 18) | ((char2 & 0x3F) << 12) | ((char3 & 0x3F) << 6) | (char4 & 0x3F))
					break
			}
		}

		return output.map(c => String.fromCodePoint(c)).join('')
	}
}

function utf8_to_ascii(input) {
	const S = String.fromCodePoint
	const K = input.charCodeAt.bind(input)
	let i = 0, output = ''
	while (input[i]) {
		let char = K(i++)
		if (char > 0xff || (char & (1 << 7))) {
			if (char >> 13) {
				output += S((char >> 18 & 0b111) | (0b1111 << 4)) // 4
			}
			if (char >> 7) {
				output += S((char >> 12 & 0b1111) | (0b111 << 5)) // 3
			}
			output += S((char >> 6 & 0b111111) | (0b1 << 7)) // 2
			output += S((char >> 0 & 0b111111) | (0b1 << 7)) // 1
		}
		else output += S(char)
	}
	return output
}

function ascii_to_utf8(input) {
	const S = String.fromCodePoint
	const K = input.charCodeAt.bind(input)
	let i = 0, char, char2, char3, char4, output = ''
	while (input[i]) {
		let char = K(i++)
		switch (char >> 4) {
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				// 0xxxxxxx
				output += S(char)
				break
			case 12:
			case 13:
				// 110x xxxx   10xx xxxx
				char2 = K(i++)
				output += S(((char & 0x1F) << 6) | (char2 & 0x3F))
				break
			case 14:
				// 1110 xxxx  10xx xxxx  10xx xxxx
				char2 = K(i++)
				char3 = K(i++)
				output += S(((char & 0x0F) << 12) | ((char2 & 0x3F) << 6) | ((char3 & 0x3F) << 0))
				break
			case 15:
				// 1111 0xxx 10xx xxxx 10xx xxxx 10xx xxxx
				char2 = K(i++)
				char3 = K(i++)
				char4 = K(i++)
				output += S(((char & 0x07) << 18) | ((char2 & 0x3F) << 12) | ((char3 & 0x3F) << 6) | (char4 & 0x3F))
				break
		}
	}
	return output
}


function utf8_to_ascii_raw(input) {
	const K = input.charCodeAt.bind(input)
	let i = 0, output = []
	while (input[i]) {
		let char = K(i++)
		if (char > 0xff || (char & (1 << 7))) {
			if (char >> 13) {
				output.push((char >> 18 & 0b111) | (0b1111 << 4)) // 4
			}
			if (char >> 7) {
				output.push((char >> 12 & 0b1111) | (0b111 << 5)) // 3
			}
			output.push((char >> 6 & 0b111111) | (0b1 << 7)) // 2
			output.push((char >> 0 & 0b111111) | (0b1 << 7)) // 1
		}
		else output.push(char)
	}
	return output
}

function ascii_to_utf8_raw(input) {
	const K = input.charCodeAt.bind(input)
	let i = 0, char, char2, char3, char4, output = []
	while (input[i]) {
		let char = K(i++)
		switch (char >> 4) {
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				// 0xxxxxxx
				output.push(char)
				break
			case 12:
			case 13:
				// 110x xxxx   10xx xxxx
				char2 = K(i++)
				output.push(((char & 0x1F) << 6) | (char2 & 0x3F))
				break
			case 14:
				// 1110 xxxx  10xx xxxx  10xx xxxx
				char2 = K(i++)
				char3 = K(i++)
				output.push(((char & 0x0F) << 12) | ((char2 & 0x3F) << 6) | ((char3 & 0x3F) << 0))
				break
			case 15:
				// 1111 0xxx 10xx xxxx 10xx xxxx 10xx xxxx
				char2 = K(i++)
				char3 = K(i++)
				char4 = K(i++)
				output.push(((char & 0x07) << 18) | ((char2 & 0x3F) << 12) | ((char3 & 0x3F) << 6) | (char4 & 0x3F))
				break
		}
	}
	return output
}
