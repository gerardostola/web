function fillOne(id, containerId) {
    const url = `dyn?id=${id}`;
    
    fetch(url)
        .then(response => {
            if (!response.ok) {
                throw new Error('Error loading content.');
            }
            return response.text();
        })
        .then(data => {
            const contenedor = document.getElementById(containerId);
            contenedor.innerHTML = data;
        })
        .catch(error => {
            console.error('Error:', error);
            const contenedor = document.getElementById(containerId);
            contenedor.innerHTML = '<p>Error loading content.</p>';
        });
}

function fillAll() {
	 const checkboxes = document.querySelectorAll('input[type="checkbox"]');

    checkboxes.forEach(checkbox => {
  	    const id = checkbox.id;
				const url = `dyn?id=${id}`;
        fetch(url)
            .then(response => {
                if (!response.ok) {
                    throw new Error('checkbox request error');
                }
                return response.json();
            })
            .then(data => {
                checkbox.checked = data.checked;
            })
            .catch(error => {
                console.error(`check ${id}:`, error);
            });
    });
	
    const selects = document.querySelectorAll('select');
    selects.forEach(select => {
				const id = select.id; 
        const url = `dyn?id=${id}`;
        fetch(url)
            .then(response => {
                if (!response.ok) {
                    throw new Error('Error on select options.');
                }
                return response.json();
            })
            .then(data => {
                select.innerHTML = '';

                data.opt.forEach(opcion => {
                    const optionElement = document.createElement('option');
                    optionElement.value = opcion.value;
                    optionElement.textContent = opcion.text;

                    if (opcion.value === data.sel) {
                        optionElement.selected = true;
                    }

                    select.appendChild(optionElement);
                });
            })
            .catch(error => {
                console.error(`Error sel ${id}:`, error);
                select.innerHTML = '<option value="">Error loading options</option>';
            });
    }); 
}


function fillPost() {

		const allIds = Array.from(document.querySelectorAll('input[type="checkbox"][id], select[id], input[type="text"][id]'))
				.map(element => parseInt(element.id, 10));

		const requestData = {
				get: allIds
		};
	fetch('dyn', {
    method: 'POST',
    headers: {
        'Content-Type': 'application/json'
    },
    body: JSON.stringify(requestData)
	})
    .then(response => {
        if (!response.ok) {
            throw new Error('Error getting server data');
        }
        return response.json();
    })
    .then(data => {
        data.forEach(item => {
            const element = document.getElementById(item.id);
            if (element) {
                if (element.tagName === 'INPUT' && element.type === 'checkbox') {
                    element.checked = item.checked;
								} else if (element.tagName === 'INPUT' && element.type === 'text') {
                    element.value = item.value || '';
								} else if (element.tagName === 'SELECT') {
                    element.innerHTML = '';
                    item.opt.forEach(opcion => {
                        const optionElement = document.createElement('option');
                        optionElement.value = opcion.value;
                        optionElement.textContent = opcion.text;

                        if (opcion.value === item.sel) {
                            optionElement.selected = true;
                        }

                        element.appendChild(optionElement);
                    });
                }
            }
        });
    })
    .catch(error => {
        console.error('Data error:', error);
    });
}

function enviarFormulario(form) {
    const formData = new FormData(form);
    const formattedData = { set: [] };
		const uniqueFields = new Map();

    formData.forEach((value, key) => {
        uniqueFields.set(key, value);
    });

    uniqueFields.forEach((value, key) => {
				const idNumber = parseInt(key, 10);
        formattedData.set.push({ id: idNumber, value: value });
    });

		const action = form.getAttribute('action');
		
    fetch(action, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(formattedData)
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Error in server response');
        }
        return response.json();
    })
    .then(data => {
        if (data.success) {
            mostrarMensaje('Form sent.', 'success');
        } else {
            mostrarMensaje('Form not sent.', 'error');
        }
    })
    .catch(error => {
        mostrarMensaje('Connection error.', 'error');
    });
}

function mostrarMensaje(mensaje, tipo) {
    const mensajeDiv = document.createElement('div');
    mensajeDiv.className = `mensaje ${tipo}`;
    mensajeDiv.textContent = mensaje;
    document.body.appendChild(mensajeDiv);
    setTimeout(() => {
        mensajeDiv.remove();
    }, 3000);
}


document.addEventListener('DOMContentLoaded', () => {
		fillPost();
    
		const forms = document.querySelectorAll('form');
    forms.forEach(form => {
        form.addEventListener('submit', function(event) {
            event.preventDefault();
            enviarFormulario(form);
        });
    });
});
