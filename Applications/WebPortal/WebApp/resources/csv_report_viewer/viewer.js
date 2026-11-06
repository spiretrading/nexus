import { ClientSideRowModelModule, createGrid, themeQuartz, TooltipModule } from
  'ag-grid-community';
import Papa from 'papaparse';

function create(container, source) {
  const file = new URL(source, document.baseURI);
  const localBlob = location.protocol === 'file:' &&
    file.protocol === 'blob:' && file.origin === 'null';
  if(!localBlob && (file.origin !== location.origin ||
      !['http:', 'https:', 'blob:'].includes(file.protocol))) {
    throw new Error('The report must be on this origin.');
  }
  const abort = new AbortController();
  let grid = null;
  let disposed = false;
  const ready = (async () => {
    const response = await fetch(file.href, {signal: abort.signal,
      credentials: 'same-origin'});
    if(!response.ok) {
      throw new Error('Unable to load CSV report.');
    }
    const text = await response.text();
    if(disposed) {
      return;
    }
    const result = Papa.parse(text, {
      delimiter: ',',
      skipEmptyLines: true,
      dynamicTyping: false
    });
    if(result.errors.length !== 0) {
      throw new Error('Unable to parse CSV report.');
    }
    const rows = result.data;
    const headers = rows.shift() || [];
    const count = rows.reduce((count, row) => Math.max(count, row.length),
      headers.length);
    const collator = new Intl.Collator(undefined, {numeric: true});
    const columns = Array.from({length: count}, (_, index) => {
      const populated = rows.some(row => row[index]?.trim());
      const numeric = populated && rows.every(row => {
        const value = row[index] || '';
        return !value.trim() || /^[+-]?(?:\d+\.?\d*|\.\d+)(?:e[+-]?\d+)?$/i.
          test(value.trim()) && Number.isFinite(Number(value)) &&
          Math.abs(Number(value)) <= Number.MAX_SAFE_INTEGER;
      });
      return {
        colId: String(index),
        headerName: headers[index] || `Column ${index + 1}`,
        valueGetter: params => params.data[index] || '',
        comparator: (left, right) => {
          if(numeric && left.trim() && right.trim()) {
            return Number(left) - Number(right);
          }
          return collator.compare(left, right);
        }
      };
    });
    grid = createGrid(container, {
      theme: themeQuartz.withParams({
        fontFamily: 'inherit',
        fontSize: 14,
        accentColor: '#684BC7',
        borderRadius: 0,
        wrapperBorderRadius: 0
      }),
      columnDefs: columns,
      rowData: rows,
      defaultColDef: {
        sortable: true,
        resizable: true,
        cellDataType: false,
        minWidth: 100,
        width: 180,
        flex: 1,
        tooltipValueGetter: params => params.value
      },
      animateRows: false,
      enableCellTextSelection: true,
      ensureDomOrder: true,
      suppressMovableColumns: true
    }, {modules: [ClientSideRowModelModule, TooltipModule]});
  })();
  return {
    ready,
    dispose() {
      if(disposed) {
        return;
      }
      disposed = true;
      abort.abort();
      grid?.destroy();
    }
  };
}

window.spireCsvViewerReady = Promise.resolve({create});
