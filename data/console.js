const version = 0x18

let ws = null
let state = {}
let config = {}
let connect_btn_status = 'Connect'

try {
	state = JSON.parse(localStorage.getItem('esp-monitor-state')) || state
} finally {
	if (state.version !== version) {
		state = {
			version,
			host: 'esp.local',
			port: 81,
			baud: 115200,
			mode: ['8', 'N', '1'],
			swap: true,
			sendTXT: true,
			sendBIN: true,
			encoding: 'txt',
			autoScroll: true,
			autoReset: true,
			historySize: 100,
			lineEnding: "NO"
		}
		save()
	}
}

setTimeout(render)
getConfig()

window.addEventListener('dragover', e => {
	e.preventDefault()
}, true)
window.addEventListener('drop', e => {
	e.preventDefault()
	const form = new FormData()
	const file = e.dataTransfer.files[0]
	const req = new XMLHttpRequest()
	form.append('file', file)
	req.open('POST', 'http://' + state.host + "/files", true)
	req.send(form)
}, true)
window.addEventListener('change', e => {
	const { target } = e;
	const bind = target.getAttribute('bind')
	const type = target.getAttribute('type')
	if (bind) {
		let value
		switch (type) {
			case 'text':
				value = `"${ target.value }"`
				break
			case 'number':
				value = `${ target.value }`
				break
			case 'checkbox':
				value = `${ target.checked }`
				break
			default:
				value = `${ target.value }`
		}
		eval(`${ bind }=${ value }`)
		console.log(bind, '=', value)
		save()
		render()
	}
}, true)

function getConfig() {
	get('config').then(cfg => {
		state.baud = cfg.bridge_baud
		state.mode = cfg.bridge_mode.split('')
		state.swap = !!cfg.bridge_swap
		render()
	})
}

function render() {
	document.querySelectorAll('[bind]').forEach(target => {
		const bind = target.getAttribute('bind')
		const type = target.getAttribute('type')
		const value = eval(bind)
		switch (type) {
			case 'checkbox':
				if (typeof value !== 'boolean')
					value = value === target.value
				target.checked = value
				break
			default:
				target.value = value
		}
	})
}
function connect() {
	if (ws) {
		ws.close()
	}
	console.log('Connecting...')
	connect_btn_status = 'Connecting...'
	ws = new WebSocket(`ws://${ state.host }:${ state.port }`)
	ws.binaryType = 'arraybuffer'
	ws.addEventListener('open', async () => {
		console.log('Connected.')
		connect_btn_status = 'Disconnect'
		render()
		getConfig()
	})
	ws.addEventListener('error', () => {
		console.log('Error.')
		connect_btn_status = 'Connect'
		render()
	})
	ws.addEventListener('close', () => {
		console.log('Closed.')
		connect_btn_status = 'Connect'
		render()
	})
	ws.addEventListener('message', (e) => {
		switch (typeof e.data) {
			case 'string':
				print(e.data, 'utf-8')
				break
			case 'object':
				print(textDecode(e.data), state.encoding)
				break
			default:
				console.warn('Unknown message data type')
		}
	})
	render()
}

const textEncode = input => new TextEncoder().encode(input)
const textDecode = input => new TextDecoder().decode(input)
const intArrayEncode = (input, radix) => new Uint8Array(input.split(' ').map(c => parseInt(c, radix)))
const intArrayDecode = (input, radix, size) => {
	return [...textEncode(input)].map(c => c.toString(radix).padStart(size, '0').toUpperCase() + ' ').join('') + ' '
}
const charDecode = (char, radix, size) => {
	return char.codePointAt(0).toString(radix).padStart(size, '0') + ' '
}
function submitMessageHandler(e) {
	switch (e.key) {
		case 'Enter': {
			e.preventDefault();
			let val = e.target.value
			switch (state.lineEnding) {
				case 'CR': val += '\n'; break
				case 'LF': val += '\r'; break
				case 'CR+LF': val += '\r\n'; break
			}
			switch (state.encoding) {
				case 'txt': val = textEncode(val); break
				case 'hex': val = intArrayEncode(val, 16); break
				case 'dec': val = intArrayEncode(val, 10); break
				case 'bin': val = intArrayEncode(val, 2); break
			}
			if (ws && ws.readyState === ws.OPEN) {
				if (!val) break
				if (state.sendBIN) {
					ws.send(val)
				}
				if (state.sendTXT) {
					ws.send(textDecode(val))
				}
			}
			e.target.value = ''
			break;
		}
		case 'Escape': {
			e.preventDefault();
			break;
		}
	}
}
function connectButtonHandler() {
	switch (connect_btn_status) {
		case 'Connect':
		case 'Reconnect':
		case 'Retry':
			connect()
			break
		case 'Disconnect':
			ws.close()
			break
	}
}

function clearOutput() {
	document.querySelector('output').innerText = ''
}
async function resetAVR() {
	await post('command/reset')
}
async function configSerial() {
	await post('config', {
		bridge_baud: state.baud,
		bridge_swap: state.swap,
		bridge_mode: state.mode.join('')
	})
	if (state.autoReset) {
		resetAVR()
	}
}
async function detectBaudrate() {
	let baud = await post('command/detect_baudrate')
	if (baud) {
		state.baud = baud
		render()
		configSerial()
		return true
	}
	return false
}

let line
const output = document.querySelector('output')
const newLine = () => {
	line = document.createElement('line')
	output.appendChild(line)
}
function print(str, encoding) {
	let i = 0, char
	if (!output.children.length) newLine()
	while (char = str[i++]) {
		if (char === '\n') {
			newLine()
			continue
		}
		if (char === '\r') {
			continue
		}
		if (!line.innerText)
			line.innerText = ''
		switch (encoding) {
			case 'hex': char = intArrayDecode(char,16, 2); break
			case 'dec': char = intArrayDecode(char,10, 3); break
			case 'bin': char = intArrayDecode(char,02, 8); break
		}
		line.innerText += char
	}
	if (output.children.length > state.historySize) {
		output.removeChild(output.children[0])
	}
	if (state.autoScroll) {
		line.scrollIntoView()
	}
}

function save() {
	localStorage.setItem('esp-monitor-state', JSON.stringify(state))
}
