const version = 0x15

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
			port: 88,
			baud: 115200,
			mode: ['8', 'N', '1'],
			swap: true,
			encode: 'TXT',
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
		state.baud = cfg.serial_baud
		state.mode = cfg.serial_mode.split('')
		state.swap = !!cfg.serial_swap
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
				print(e.data)
				break
			case 'object':
				handleBinary(e.data)
				break
		}
	})
	render()
}

function submitMessageHandler(e) {
	switch (e.key) {
		case 'Enter': {
			e.preventDefault();
			if (ws && ws.readyState === ws.OPEN) {
				let val = e.target.value
				switch (state.encode) {
					case 'HEX':
						val = val.trim().split(/\s+/).map(i => String.fromCharCode(parseInt(i, 16))).join('')
						break
					case 'BIN':
						val = val.trim().split(/\s+/).map(i => String.fromCharCode(parseInt(i, 2))).join('')
						break
					case 'DEC':
						val = val.trim().split(/\s+/).map(i => String.fromCharCode(parseInt(i, 10))).join('')
						break
				}
				switch (state.lineEnding) {
					case 'CR': val = val + '\r'; break
					case 'NL': val = val + '\n'; break
					case 'CR+NL': val = val + '\r\n'; break
				}
				ws.send(val)
				e.target.value = ''
			}
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
		serial_baud: state.baud,
		serial_swap: state.swap,
		serial_mode: state.mode.join('')
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

function print(str) {
	const output = document.querySelector('output')
	str = str.replace(/(\r|\n)+/g, '\n')
	if (!output.childNodes[0]) {
		output.append(str)
	} else {
		output.childNodes[output.childNodes.length - 1].appendData(str)
		if (str.endsWith('\n')) {
			output.append('')
		}
	}
	if (output.childNodes.length > state.historySize) {
		output.removeChild(output.childNodes[0])
	}
	if (state.autoScroll) {
		output.scrollTop = output.scrollHeight
	}
}
function handleBinary(buf) {
	let output = '';
	const view = new DataView(buf)
	for (let i = 0 ; i < buf.byteLength ; i++) {
		const code = view.getUint8(i);
		let char;
		switch (state.encode) {
			case 'DEC':
				char = code.toString(10);
				// if (char.length == 1) char = '00' + char
				// else if (char.length == 2) char = '0' + char
				if (code === 10) char += '\n'
				else char += ' '
				break
			case 'HEX':
				char = code.toString(16).toUpperCase();
				if (char.length == 1) char = '0' + char
				if (code === 10) char += '\n'
				else char += ' '
				break
			case 'BIN':
				char = (256 | code).toString(2).slice(1, 9);
				if (code === 10) char += '\n'
				else char += ' '
				break
			case 'TXT':
				char = decoder.write(code);
				break
		}
		output += char
	}
	print(output)
}



function save() {
	localStorage.setItem('esp-monitor-state', JSON.stringify(state))
}
