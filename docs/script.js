const dataPath = './chemicals/chemicals.json';
const grid = document.querySelector('#chemicals');
const searchInput = document.querySelector('#search');
const template = document.querySelector('#chemical-card');

async function fetchChemicals() {
  const response = await fetch(dataPath);
  if (!response.ok) {
    throw new Error('Unable to load chemical data');
  }
  return response.json();
}

function renderChemicals(chemicals) {
  grid.innerHTML = '';
  chemicals.forEach((chem) => {
    const card = template.content.cloneNode(true);
    card.querySelector('.name').textContent = chem.name;
    card.querySelector('.formula').textContent = chem.formula;
    card.querySelector('.description').textContent = chem.description;
    card.querySelector('.cas').textContent = chem.cas;
    card.querySelector('.hazards').textContent = chem.hazards.join(', ');
    card.querySelector('.uses').textContent = chem.uses.join(', ');
    card.querySelector('.storage').textContent = chem.storage;
    grid.appendChild(card);
  });
}

function filterChemicals(chemicals, query) {
  const q = query.trim().toLowerCase();
  if (!q) return chemicals;

  return chemicals.filter((chem) => {
    const haystack = [
      chem.name,
      chem.formula,
      chem.cas,
      ...(chem.hazards || []),
      ...(chem.uses || []),
    ]
      .join(' ')
      .toLowerCase();

    return haystack.includes(q);
  });
}

(async () => {
  try {
    const chemicals = await fetchChemicals();
    renderChemicals(chemicals);

    searchInput.addEventListener('input', (event) => {
      const filtered = filterChemicals(chemicals, event.target.value);
      renderChemicals(filtered);
    });
  } catch (error) {
    grid.innerHTML = `<p class="error">${error.message}</p>`;
  }
})();
