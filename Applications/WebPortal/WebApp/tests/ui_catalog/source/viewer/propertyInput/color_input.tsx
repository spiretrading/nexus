import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value: string;

  /** The callback to update the value. */
  update?: (newValue: any) => void;
}

interface State {
  hexValue: string;
  r: string;
  g: string;
  b: string;
  showPicker: boolean;
  hue: number;
  saturation: number;
  brightness: number;
}

const PALETTE = [
  '#000000', '#333333', '#666666', '#999999', '#CCCCCC', '#FFFFFF',
  '#FF0000', '#FF4444', '#FF8888', '#FFCCCC', '#CC0000', '#880000',
  '#FF8800', '#FFAA44', '#FFCC88', '#FFEECC', '#CC6600', '#884400',
  '#FFFF00', '#FFFF44', '#FFFF88', '#FFFFCC', '#CCCC00', '#888800',
  '#00FF00', '#44FF44', '#88FF88', '#CCFFCC', '#00CC00', '#008800',
  '#00FFFF', '#44FFFF', '#88FFFF', '#CCFFFF', '#00CCCC', '#008888',
  '#0000FF', '#4444FF', '#8888FF', '#CCCCFF', '#0000CC', '#000088',
  '#8800FF', '#AA44FF', '#CC88FF', '#EECCFF', '#6600CC', '#440088',
  '#FF00FF', '#FF44FF', '#FF88FF', '#FFCCFF', '#CC00CC', '#880088'
];

const SV_WIDTH = 200;
const SV_HEIGHT = 150;
const HUE_HEIGHT = 16;

function hsvToRgb(
    h: number, s: number, v: number): {r: number, g: number, b: number} {
  const c = v * s;
  const x = c * (1 - Math.abs((h / 60) % 2 - 1));
  const m = v - c;
  let r1: number, g1: number, b1: number;
  if(h < 60) { r1 = c; g1 = x; b1 = 0; }
  else if(h < 120) { r1 = x; g1 = c; b1 = 0; }
  else if(h < 180) { r1 = 0; g1 = c; b1 = x; }
  else if(h < 240) { r1 = 0; g1 = x; b1 = c; }
  else if(h < 300) { r1 = x; g1 = 0; b1 = c; }
  else { r1 = c; g1 = 0; b1 = x; }
  return {
    r: Math.round((r1 + m) * 255),
    g: Math.round((g1 + m) * 255),
    b: Math.round((b1 + m) * 255)
  };
}

function rgbToHsv(
    r: number, g: number, b: number): {h: number, s: number, v: number} {
  const r1 = r / 255, g1 = g / 255, b1 = b / 255;
  const max = Math.max(r1, g1, b1);
  const min = Math.min(r1, g1, b1);
  const d = max - min;
  let h = 0;
  if(d !== 0) {
    if(max === r1) { h = 60 * (((g1 - b1) / d) % 6); }
    else if(max === g1) { h = 60 * (((b1 - r1) / d) + 2); }
    else { h = 60 * (((r1 - g1) / d) + 4); }
  }
  if(h < 0) { h += 360; }
  return { h, s: max === 0 ? 0 : d / max, v: max };
}

function hexToRgb(hex: string): {r: number, g: number, b: number} | null {
  const clean = hex.replace('#', '');
  if(clean.length !== 6 || !/^[0-9A-Fa-f]{6}$/.test(clean)) {
    return null;
  }
  return {
    r: parseInt(clean.substring(0, 2), 16),
    g: parseInt(clean.substring(2, 4), 16),
    b: parseInt(clean.substring(4, 6), 16)
  };
}

function rgbToHex(r: number, g: number, b: number): string {
  const clamp = (v: number) => Math.max(0, Math.min(255, Math.round(v)));
  const toHex = (v: number) =>
    clamp(v).toString(16).padStart(2, '0').toUpperCase();
  return `#${toHex(r)}${toHex(g)}${toHex(b)}`;
}

function normalizeToHex(color: string): string {
  const style = new Option().style;
  style.color = color;
  const computed = style.color;
  if(!computed) {
    return '#000000';
  }
  const rgbMatch = computed.match(/rgba?\((\d+),\s*(\d+),\s*(\d+)/);
  if(rgbMatch) {
    return rgbToHex(
      parseInt(rgbMatch[1]), parseInt(rgbMatch[2]), parseInt(rgbMatch[3]));
  }
  if(computed.startsWith('#')) {
    return computed.toUpperCase();
  }
  return '#000000';
}

function stateFromHex(hex: string): Partial<State> {
  const rgb = hexToRgb(hex);
  const r = rgb?.r ?? 0;
  const g = rgb?.g ?? 0;
  const b = rgb?.b ?? 0;
  const hsv = rgbToHsv(r, g, b);
  return {
    hexValue: hex,
    r: String(r),
    g: String(g),
    b: String(b),
    hue: hsv.h,
    saturation: hsv.s,
    brightness: hsv.v
  };
}

/** A color input with a dropdown picker containing an SV rectangle,
 *  hue slider, and predefined color palette. */
export class ColorInput extends React.Component<Properties, State> {
  private svCanvasRef = React.createRef<HTMLCanvasElement>();
  private hueCanvasRef = React.createRef<HTMLCanvasElement>();
  private containerRef = React.createRef<HTMLDivElement>();
  private draggingSV = false;
  private draggingHue = false;

  constructor(props: Properties) {
    super(props);
    const hex = normalizeToHex(this.props.value);
    this.state = {
      showPicker: false,
      ...stateFromHex(hex)
    } as State;
  }

  public render(): JSX.Element {
    return (
      <div style={ColorInput.STYLE.container} ref={this.containerRef}>
        <div style={ColorInput.STYLE.previewRow}>
          <div style={{backgroundColor: this.props.value,
              ...ColorInput.STYLE.preview}}
            onClick={this.togglePicker}/>
          <div style={ColorInput.STYLE.hexRow}>
            <span style={ColorInput.STYLE.label}>HEX</span>
            <input style={ColorInput.STYLE.hexInput}
              value={this.state.hexValue}
              onChange={this.onHexChange}
              onBlur={this.onHexBlur}/>
          </div>
        </div>
        <div style={ColorInput.STYLE.rgbRow}>
          <div style={ColorInput.STYLE.rgbField}>
            <span style={ColorInput.STYLE.label}>R</span>
            <input style={ColorInput.STYLE.rgbInput} type='number'
              min={0} max={255} value={this.state.r}
              onChange={this.onRChange} onBlur={this.onRgbBlur}/>
          </div>
          <div style={ColorInput.STYLE.rgbField}>
            <span style={ColorInput.STYLE.label}>G</span>
            <input style={ColorInput.STYLE.rgbInput} type='number'
              min={0} max={255} value={this.state.g}
              onChange={this.onGChange} onBlur={this.onRgbBlur}/>
          </div>
          <div style={ColorInput.STYLE.rgbField}>
            <span style={ColorInput.STYLE.label}>B</span>
            <input style={ColorInput.STYLE.rgbInput} type='number'
              min={0} max={255} value={this.state.b}
              onChange={this.onBChange} onBlur={this.onRgbBlur}/>
          </div>
        </div>
        {this.state.showPicker && this.renderPicker()}
      </div>);
  }

  public componentDidMount() {
    document.addEventListener('mousedown', this.onClickOutside);
    document.addEventListener('mousemove', this.onMouseMove);
    document.addEventListener('mouseup', this.onMouseUp);
  }

  public componentWillUnmount() {
    document.removeEventListener('mousedown', this.onClickOutside);
    document.removeEventListener('mousemove', this.onMouseMove);
    document.removeEventListener('mouseup', this.onMouseUp);
  }

  public componentDidUpdate(prevProps: Properties, prevState: State) {
    if(this.props.value !== prevProps.value) {
      const hex = normalizeToHex(this.props.value);
      this.setState(stateFromHex(hex) as State);
    }
    if(this.state.showPicker &&
        (!prevState.showPicker || this.state.hue !== prevState.hue)) {
      this.drawSVCanvas();
      this.drawHueCanvas();
    }
  }

  private renderPicker(): JSX.Element {
    const svX = this.state.saturation * SV_WIDTH;
    const svY = (1 - this.state.brightness) * SV_HEIGHT;
    const hueX = (this.state.hue / 360) * SV_WIDTH;
    return (
      <div style={ColorInput.STYLE.picker}>
        <div style={ColorInput.STYLE.svContainer}>
          <canvas ref={this.svCanvasRef}
            width={SV_WIDTH} height={SV_HEIGHT}
            style={ColorInput.STYLE.canvas}
            onMouseDown={this.onSVMouseDown}/>
          <div style={{
            ...ColorInput.STYLE.svCursor,
            left: `${svX - 6}px`,
            top: `${svY - 6}px`}}/>
        </div>
        <div style={ColorInput.STYLE.hueContainer}>
          <canvas ref={this.hueCanvasRef}
            width={SV_WIDTH} height={HUE_HEIGHT}
            style={ColorInput.STYLE.canvas}
            onMouseDown={this.onHueMouseDown}/>
          <div style={{
            ...ColorInput.STYLE.hueCursor,
            left: `${hueX - 2}px`}}/>
        </div>
        <div style={ColorInput.STYLE.palette}>
          {PALETTE.map(color =>
            <button key={color}
              style={{backgroundColor: color,
                ...ColorInput.STYLE.swatch,
                ...(this.props.value === color &&
                  ColorInput.STYLE.swatchSelected)}}
              onClick={() => this.onPaletteSelect(color)}/>)}
        </div>
      </div>);
  }

  private drawSVCanvas() {
    const canvas = this.svCanvasRef.current;
    if(!canvas) { return; }
    const ctx = canvas.getContext('2d');
    const {h} = {h: this.state.hue};
    const pureColor = hsvToRgb(h, 1, 1);
    ctx.fillStyle =
      `rgb(${pureColor.r}, ${pureColor.g}, ${pureColor.b})`;
    ctx.fillRect(0, 0, SV_WIDTH, SV_HEIGHT);
    const whiteGrad = ctx.createLinearGradient(0, 0, SV_WIDTH, 0);
    whiteGrad.addColorStop(0, 'rgba(255, 255, 255, 1)');
    whiteGrad.addColorStop(1, 'rgba(255, 255, 255, 0)');
    ctx.fillStyle = whiteGrad;
    ctx.fillRect(0, 0, SV_WIDTH, SV_HEIGHT);
    const blackGrad = ctx.createLinearGradient(0, 0, 0, SV_HEIGHT);
    blackGrad.addColorStop(0, 'rgba(0, 0, 0, 0)');
    blackGrad.addColorStop(1, 'rgba(0, 0, 0, 1)');
    ctx.fillStyle = blackGrad;
    ctx.fillRect(0, 0, SV_WIDTH, SV_HEIGHT);
  }

  private drawHueCanvas() {
    const canvas = this.hueCanvasRef.current;
    if(!canvas) { return; }
    const ctx = canvas.getContext('2d');
    const grad = ctx.createLinearGradient(0, 0, SV_WIDTH, 0);
    for(let i = 0; i <= 6; i++) {
      grad.addColorStop(i / 6, `hsl(${i * 60}, 100%, 50%)`);
    }
    ctx.fillStyle = grad;
    ctx.fillRect(0, 0, SV_WIDTH, HUE_HEIGHT);
  }

  private togglePicker = () => {
    this.setState(prev => ({showPicker: !prev.showPicker}));
  }

  private onClickOutside = (event: MouseEvent) => {
    if(this.state.showPicker && this.containerRef.current &&
        !this.containerRef.current.contains(event.target as Node)) {
      this.setState({showPicker: false});
    }
  }

  private onSVMouseDown = (event: React.MouseEvent<HTMLCanvasElement>) => {
    this.draggingSV = true;
    this.updateSV(event.nativeEvent);
  }

  private onHueMouseDown = (event: React.MouseEvent<HTMLCanvasElement>) => {
    this.draggingHue = true;
    this.updateHue(event.nativeEvent);
  }

  private onMouseMove = (event: MouseEvent) => {
    if(this.draggingSV) {
      this.updateSV(event);
    } else if(this.draggingHue) {
      this.updateHue(event);
    }
  }

  private onMouseUp = () => {
    this.draggingSV = false;
    this.draggingHue = false;
  }

  private updateSV(event: MouseEvent) {
    const canvas = this.svCanvasRef.current;
    if(!canvas) { return; }
    const rect = canvas.getBoundingClientRect();
    const s = Math.max(0, Math.min(1, (event.clientX - rect.left) / SV_WIDTH));
    const v = Math.max(0,
      Math.min(1, 1 - (event.clientY - rect.top) / SV_HEIGHT));
    this.setState({saturation: s, brightness: v});
    const rgb = hsvToRgb(this.state.hue, s, v);
    const hex = rgbToHex(rgb.r, rgb.g, rgb.b);
    this.setState({
      hexValue: hex,
      r: String(rgb.r), g: String(rgb.g), b: String(rgb.b)
    });
    this.props.update(hex);
  }

  private updateHue(event: MouseEvent) {
    const canvas = this.hueCanvasRef.current;
    if(!canvas) { return; }
    const rect = canvas.getBoundingClientRect();
    const h = Math.max(0,
      Math.min(360, ((event.clientX - rect.left) / SV_WIDTH) * 360));
    this.setState({hue: h});
    const rgb = hsvToRgb(h, this.state.saturation, this.state.brightness);
    const hex = rgbToHex(rgb.r, rgb.g, rgb.b);
    this.setState({
      hexValue: hex,
      r: String(rgb.r), g: String(rgb.g), b: String(rgb.b)
    });
    this.props.update(hex);
  }

  private onPaletteSelect = (color: string) => {
    this.props.update(color);
  }

  private onHexChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const value = event.target.value;
    this.setState({hexValue: value});
    const rgb = hexToRgb(value);
    if(rgb) {
      const hsv = rgbToHsv(rgb.r, rgb.g, rgb.b);
      this.setState({
        r: String(rgb.r), g: String(rgb.g), b: String(rgb.b),
        hue: hsv.h, saturation: hsv.s, brightness: hsv.v
      });
      this.props.update(value.startsWith('#') ? value : `#${value}`);
    }
  }

  private onHexBlur = () => {
    const rgb = hexToRgb(this.state.hexValue);
    if(!rgb) {
      const hex = normalizeToHex(this.props.value);
      this.setState(stateFromHex(hex) as State);
    }
  }

  private onRChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.setState({r: event.target.value});
    this.commitRgb(event.target.value, this.state.g, this.state.b);
  }

  private onGChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.setState({g: event.target.value});
    this.commitRgb(this.state.r, event.target.value, this.state.b);
  }

  private onBChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.setState({b: event.target.value});
    this.commitRgb(this.state.r, this.state.g, event.target.value);
  }

  private onRgbBlur = () => {
    const r = ColorInput.clampChannel(this.state.r);
    const g = ColorInput.clampChannel(this.state.g);
    const b = ColorInput.clampChannel(this.state.b);
    const hex = rgbToHex(r, g, b);
    const hsv = rgbToHsv(r, g, b);
    this.setState({
      r: String(r), g: String(g), b: String(b),
      hexValue: hex,
      hue: hsv.h, saturation: hsv.s, brightness: hsv.v
    });
    this.props.update(hex);
  }

  private commitRgb(rStr: string, gStr: string, bStr: string) {
    const r = parseInt(rStr);
    const g = parseInt(gStr);
    const b = parseInt(bStr);
    if(!isNaN(r) && !isNaN(g) && !isNaN(b)) {
      const hex = rgbToHex(r, g, b);
      const hsv = rgbToHsv(
        Math.max(0, Math.min(255, r)),
        Math.max(0, Math.min(255, g)),
        Math.max(0, Math.min(255, b)));
      this.setState({
        hexValue: hex,
        hue: hsv.h, saturation: hsv.s, brightness: hsv.v
      });
      this.props.update(hex);
    }
  }

  private static clampChannel(value: string): number {
    const n = parseInt(value);
    if(isNaN(n)) {
      return 0;
    }
    return Math.max(0, Math.min(255, n));
  }

  private static readonly STYLE = {
    container: {
      width: '100%',
      display: 'flex',
      flexDirection: 'column',
      gap: '8px',
      position: 'relative'
    } as React.CSSProperties,
    previewRow: {
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      gap: '10px'
    } as React.CSSProperties,
    preview: {
      width: '36px',
      height: '36px',
      borderRadius: '4px',
      border: '1px solid #C8C8C8',
      flexShrink: 0,
      cursor: 'pointer'
    } as React.CSSProperties,
    hexRow: {
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      gap: '6px',
      flexGrow: 1
    } as React.CSSProperties,
    label: {
      fontSize: '12px',
      fontWeight: 500,
      color: '#333333',
      flexShrink: 0,
      minWidth: '16px'
    } as React.CSSProperties,
    hexInput: {
      width: '100%',
      boxSizing: 'border-box',
      fontFamily: 'monospace',
      fontSize: '13px'
    } as React.CSSProperties,
    rgbRow: {
      display: 'flex',
      flexDirection: 'row',
      gap: '8px'
    } as React.CSSProperties,
    rgbField: {
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      gap: '4px',
      flex: 1
    } as React.CSSProperties,
    rgbInput: {
      width: '100%',
      boxSizing: 'border-box',
      fontSize: '13px'
    } as React.CSSProperties,
    picker: {
      position: 'absolute',
      top: '100%',
      left: 0,
      zIndex: 1000,
      backgroundColor: '#FFFFFF',
      border: '1px solid #C8C8C8',
      borderRadius: '4px',
      padding: '10px',
      boxShadow: '0 4px 12px rgba(0, 0, 0, 0.15)',
      display: 'flex',
      flexDirection: 'column',
      gap: '8px'
    } as React.CSSProperties,
    svContainer: {
      position: 'relative',
      width: `${SV_WIDTH}px`,
      height: `${SV_HEIGHT}px`,
      cursor: 'crosshair'
    } as React.CSSProperties,
    canvas: {
      display: 'block',
      borderRadius: '2px'
    } as React.CSSProperties,
    svCursor: {
      position: 'absolute',
      width: '12px',
      height: '12px',
      borderRadius: '50%',
      border: '2px solid #FFFFFF',
      boxShadow: '0 0 2px rgba(0, 0, 0, 0.6)',
      pointerEvents: 'none'
    } as React.CSSProperties,
    hueContainer: {
      position: 'relative',
      width: `${SV_WIDTH}px`,
      height: `${HUE_HEIGHT}px`,
      cursor: 'crosshair'
    } as React.CSSProperties,
    hueCursor: {
      position: 'absolute',
      top: '-1px',
      width: '4px',
      height: `${HUE_HEIGHT + 2}px`,
      backgroundColor: '#FFFFFF',
      border: '1px solid #666666',
      borderRadius: '1px',
      pointerEvents: 'none'
    } as React.CSSProperties,
    palette: {
      display: 'flex',
      flexWrap: 'wrap',
      gap: '3px',
      maxWidth: `${SV_WIDTH}px`
    } as React.CSSProperties,
    swatch: {
      width: '18px',
      height: '18px',
      border: '1px solid #C8C8C8',
      borderRadius: '2px',
      cursor: 'pointer',
      padding: 0,
      appearance: 'none' as const
    } as React.CSSProperties,
    swatchSelected: {
      outline: '2px solid #4B23A0',
      outlineOffset: '1px'
    } as React.CSSProperties
  };
}
