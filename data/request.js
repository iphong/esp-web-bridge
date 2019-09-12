function post(path, params) {
	return request(path, params, 'POST')
}
function get(path, params) {
	return request(path, params, 'GET')
}
async function request(path, params = {}, method, data) {
	return new Promise((resolve = console.log, reject = console.warn) => {
		const req = new XMLHttpRequest()
		const query = Object.entries(params).map(([name, value]) => {
			if (typeof value === 'boolean') {
				value = value ? '1' : '0'
			}
			return `${ name }=${ value }`
		}).join('&')
		req.open(method, `http://${ state.host }/${ path }${ query ? '?' + query : '' }`, true)
		req.addEventListener('loadend', e => {
			let responseValue = req.responseText
			const contentType = req.getResponseHeader('Content-Type')
			if (contentType === 'application/json') {
				try {
					responseValue = JSON.parse(req.responseText)
				} finally {
					resolve(responseValue)
				}
			} else resolve(responseValue)
		})
		req.addEventListener('error', reject)
		req.send(data)
	})
}
