import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { QueryModel } from '../../models';
import { Button } from '../button';
import { Input } from '../input';
import { parseCsv } from './csv';

interface Properties<T> {

  /** The title of the editor. */
  title: string;

  /** The title in single selection mode. Falls back to title. */
  titleSingle?: string;

  /** The heading above the selected items. */
  listHeading: string;

  /** The heading in single selection mode. Falls back to listHeading. */
  listHeadingSingle?: string;

  /** Resolves suggestions, typed entries, and CSV entries. */
  model: QueryModel<T>;

  /** The committed selection. Changes reset the editor's working selection. */
  selected: readonly T[];

  /** Returns the text displayed for an item. */
  getLabel: (item: T) => string;

  /** Compares items for selection and deduplication.
   *  Defaults to Set equality. */
  isEqual?: (first: T, second: T) => boolean;

  /** The number of items that can be selected. Defaults to MULTIPLE. */
  selectionMode?: EditListModal.SelectionMode;

  /** Whether the selection can only be viewed. */
  readOnly?: boolean;

  /** The prompt displayed in the item search. */
  placeholder?: string;

  /** Called with the edited selection when Submit is pressed. */
  onSubmit?: (selected: T[]) => void;

  /** Called on dismissal. The caller should unmount the modal. */
  onClose?: () => void;
}

interface State<T> {
  selected: T[];
  submission: T[];
  query: string;
  highlighted: number;
  suggestionSelected: boolean;
  removal: number;
  expanded: boolean;
  error: string;
  importing: boolean;
  adding: boolean;
  suggestions: readonly T[];
  searching: boolean;
  searchError: string;
}

/** Edits a selection from a list of choices without changing the caller's data
 *  until submission. CSV files may contain comma-separated or newline-separated
 *  queries resolved by the model; unresolved entries reject the entire import.
 */
export class EditListModal<T> extends React.Component<Properties<T>, State<T>> {
  constructor(props: Properties<T>) {
    super(props);
    const selected = this.normalize(props.selected);
    this.state = {
      selected,
      submission: selected.slice(),
      query: '',
      highlighted: 0,
      suggestionSelected: false,
      removal: -1,
      expanded: false,
      error: '',
      importing: false,
      adding: false,
      suggestions: [],
      searching: false,
      searchError: ''
    };
    this.dialog = React.createRef<HTMLDialogElement>();
    this.input = React.createRef<HTMLInputElement>();
    this.upload = React.createRef<HTMLInputElement>();
    this.reader = null;
    this.searchVersion = 0;
    this.importVersion = 0;
    this.searchTimer = null;
    this.identifier = `edit-list-${EditListModal.nextIdentifier++}`;
  }

  public componentDidMount(): void {
    this.dialog.current.showModal();
  }

  public componentDidUpdate(previous: Properties<T>): void {
    if(previous.selected !== this.props.selected ||
        previous.selectionMode !== this.props.selectionMode ||
        previous.readOnly !== this.props.readOnly) {
      this.cancelImport();
      this.cancelSearch();
      const selected = this.normalize(this.props.selected);
      this.setState({selected, submission: selected.slice(), query: '',
        removal: -1, highlighted: 0, expanded: false, error: '',
        importing: false, adding: false, suggestions: [], searching: false,
        searchError: ''});
    } else if(previous.model !== this.props.model) {
      this.cancelImport();
      this.setState({importing: false, error: ''});
      this.search(this.state.query);
    }
    if(this.state.expanded) {
      this.dialog.current.querySelector(
        `#${this.identifier}-match-${this.state.highlighted}`)?.scrollIntoView(
          {block: 'nearest'});
    }
  }

  public componentWillUnmount(): void {
    this.cancelImport();
    this.cancelSearch();
    this.dialog.current.close();
  }

  public render(): JSX.Element {
    const single = this.isSingle();
    const heading = (() => {
      if(single) {
        return this.props.listHeadingSingle ?? this.props.listHeading;
      }
      return this.props.listHeading;
    })();
    const title = (() => {
      if(this.props.readOnly) {
        return heading;
      }
      if(single) {
        return this.props.titleSingle ?? this.props.title;
      }
      return this.props.title;
    })();
    const matches = this.getMatches();
    const changed = this.state.selected.length !==
      this.state.submission.length || this.state.selected.some((item, i) =>
        !this.equals(item, this.state.submission[i]));
    return (
      <dialog ref={this.dialog} className={css(STYLES.dialog)}
          aria-labelledby={`${this.identifier}-title`}
          onCancel={this.onCancel} onClick={this.onBackdropClick}>
        <header className={css(STYLES.header)}>
          <span id={`${this.identifier}-title`}>{title}</span>
          <button type='button' aria-label='Close'
              className={css(STYLES.close)} onClick={this.onClose}>
            <img src='resources/close.svg' width='20' height='20' alt=''/>
          </button>
        </header>
        <Content readOnly={this.props.readOnly}
            onClose={this.onClose} onSubmit={this.onSubmit}
            disabled={!changed || this.state.importing || this.state.adding}>
          <Form readOnly={this.props.readOnly}>
            {!this.props.readOnly &&
              <div className={css(STYLES.search)}>
                <Input ref={this.input} role='combobox'
                  aria-label={this.props.placeholder ?? 'Find an item'}
                  aria-autocomplete='list'
                  aria-busy={this.state.searching || this.state.adding}
                  aria-expanded={this.state.expanded}
                  aria-controls={`${this.identifier}-suggestions`}
                  aria-activedescendant={(() => {
                    if(this.state.expanded &&
                        matches[this.state.highlighted] !== undefined) {
                      return `${this.identifier}-match-${
                        this.state.highlighted}`;
                    }
                    return undefined;
                  })()}
                  autoComplete='off'
                  placeholder={this.props.placeholder ?? 'Find an item'}
                  value={this.state.query} style={{width: '100%'}}
                  onChange={this.onQuery}
                  onBlur={this.onBlur} onKeyDown={this.onKeyDown}/>
                {this.state.expanded &&
                  <ul id={`${this.identifier}-suggestions`} role='listbox'
                      aria-label='Available items'
                      className={css(STYLES.suggestions)}>
                    {matches.map((item, i) =>
                      <li key={i} role='option'
                          id={`${this.identifier}-match-${i}`}
                          aria-selected={i === this.state.highlighted}
                          className={css(STYLES.suggestion)}
                          onPointerEnter={() => this.setState({highlighted: i,
                            suggestionSelected: true})}
                          onMouseDown={this.onSuggestionMouseDown}
                          onClick={() => this.add(item)}>
                        {this.props.getLabel(item)}
                      </li>)}
                    {this.state.searching &&
                      <li role='presentation' className={css(STYLES.noMatches)}>
                        <span role='status'>Searching...</span>
                      </li>}
                    {this.state.searchError &&
                      <li role='presentation' className={css(STYLES.noMatches)}>
                        <span role='alert'>{this.state.searchError}</span>
                      </li>}
                    {matches.length === 0 && !this.state.searching &&
                        !this.state.searchError &&
                      <li role='presentation' className={css(STYLES.noMatches)}>
                        No matches
                      </li>}
                  </ul>}
              </div>}
            <Section readOnly={this.props.readOnly} single={single}
                heading={heading}>
              <ItemsList items={this.state.selected}
                getLabel={this.props.getLabel} readOnly={this.props.readOnly}
                selection={this.state.removal} onSelect={this.onSelect}/>
            </Section>
            {!this.props.readOnly &&
              <Actions single={single} removal={this.state.removal}
                importing={this.state.importing} onRemove={this.onRemove}
                onUpload={this.onUpload}/>}
            {!this.props.readOnly &&
              <input ref={this.upload} type='file' accept='.csv,text/csv'
                hidden onChange={this.onFileSelected}/>}
            {this.state.error &&
              <p role='alert' className={css(STYLES.error)}>
                {this.state.error}
              </p>}
            {this.state.importing && <p role='status'>Importing...</p>}
            {this.state.adding && <p role='status'>Adding item...</p>}
          </Form>
        </Content>
      </dialog>);
  }

  private isSingle(): boolean {
    return this.props.selectionMode === EditListModal.SelectionMode.SINGLE;
  }

  private equals(first: T, second: T): boolean {
    if(this.props.isEqual) {
      return this.props.isEqual(first, second);
    }
    return first === second || Object.is(first, second);
  }

  private normalize(items: readonly T[]): T[] {
    const result: T[] = [];
    for(const item of items) {
      if(!result.some(entry => this.equals(entry, item))) {
        result.push(item);
        if(this.isSingle()) {
          break;
        }
      }
    }
    return result;
  }

  private getMatches(): T[] {
    return this.normalizeChoices(this.state.suggestions).filter(item =>
      !this.state.selected.some(selected => this.equals(item, selected)));
  }

  private normalizeChoices(items: readonly T[]): T[] {
    if(!this.props.isEqual) {
      return Array.from(new Set(items));
    }
    return items.filter((item, i, items) =>
      items.findIndex(other => this.equals(item, other)) === i);
  }

  private add(item: T): void {
    this.cancelSearch();
    let selected = this.normalize([...this.state.selected, item]);
    if(this.isSingle()) {
      selected = [item];
    }
    this.setState({selected, query: '', highlighted: 0, removal: -1,
      error: '', expanded: false, adding: false, searching: false,
      searchError: ''});
  }

  private cancelImport(): void {
    ++this.importVersion;
    if(this.reader) {
      this.reader.onload = null;
      this.reader.onerror = null;
      this.reader.abort();
      this.reader = null;
    }
  }

  private async addQuery(fallback: T): Promise<void> {
    if(this.state.adding) {
      return;
    }
    const version = this.searchVersion;
    const query = this.state.query;
    this.setState({expanded: true, adding: true, error: ''});
    try {
      const item = await this.props.model.parse(query);
      if(version !== this.searchVersion) {
        return;
      }
      if(item !== null) {
        this.add(item);
      } else if(fallback !== null) {
        this.add(fallback);
      } else if(!this.state.searching) {
        this.setState({error: `Unknown or ambiguous item: ${query}`});
      }
    } catch {
      if(version === this.searchVersion) {
        this.setState({error: 'Unable to resolve this item. Try again.'});
      }
    } finally {
      if(version === this.searchVersion) {
        this.setState({adding: false});
      }
    }
  }

  private cancelSearch(): void {
    ++this.searchVersion;
    if(this.searchTimer !== null) {
      window.clearTimeout(this.searchTimer);
      this.searchTimer = null;
    }
  }

  private search(query: string): void {
    this.cancelSearch();
    const model = this.props.model;
    this.setState({suggestions: [], searching: false, searchError: '',
      adding: false, suggestionSelected: false});
    if(query === '' || this.props.readOnly) {
      return;
    }
    const version = this.searchVersion;
    this.setState({searching: true});
    this.searchTimer = window.setTimeout(async () => {
      this.searchTimer = null;
      try {
        const suggestions = await model.submit(query);
        if(version === this.searchVersion) {
          this.setState({suggestions, searching: false, highlighted: 0});
        }
      } catch {
        if(version === this.searchVersion) {
          this.setState({searching: false,
            searchError: 'Unable to load items. Try typing again.'});
        }
      }
    }, 200);
  }

  private onCancel = (event: React.SyntheticEvent) => {
    if(event.target !== event.currentTarget) {
      return;
    }
    event.preventDefault();
    this.onClose();
  };

  private onBackdropClick = (event: React.MouseEvent<HTMLDialogElement>) => {
    if(event.target === event.currentTarget) {
      const bounds = event.currentTarget.getBoundingClientRect();
      if(event.clientX < bounds.left || event.clientX > bounds.right ||
          event.clientY < bounds.top || event.clientY > bounds.bottom) {
        this.onClose();
      }
    }
  };

  private onClose = () => {
    this.cancelImport();
    this.cancelSearch();
    this.props.onClose?.();
  };

  private onSubmit = () => {
    if(this.state.adding || this.state.importing) {
      return;
    }
    this.setState({submission: this.state.selected.slice()});
    this.props.onSubmit?.(this.state.selected.slice());
  };

  private onQuery = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.setState({query: event.target.value, highlighted: 0,
      expanded: event.target.value !== ''});
    this.search(event.target.value);
  };

  private onBlur = () => {
    this.setState({expanded: false});
  };

  private onKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    const matches = this.getMatches();
    if(event.key === 'ArrowDown' || event.key === 'ArrowUp') {
      if(!this.state.expanded) {
        return;
      }
      event.preventDefault();
      let highlighted = this.state.highlighted;
      if(event.key === 'ArrowDown') {
        highlighted = Math.min(highlighted + 1, matches.length - 1);
      } else {
        highlighted = Math.max(highlighted - 1, 0);
      }
      this.setState({highlighted: Math.max(0, highlighted),
        suggestionSelected: matches.length !== 0});
    } else if(event.key === 'Enter') {
      event.preventDefault();
      const item = matches[this.state.highlighted];
      if(item !== undefined && this.state.expanded &&
          this.state.suggestionSelected) {
        this.add(item);
      } else if(this.state.query !== '' && !this.props.readOnly) {
        let fallback: T = null;
        if(this.state.expanded) {
          fallback = item ?? null;
        }
        this.addQuery(fallback);
      }
    } else if(event.key === 'Escape' && this.state.expanded) {
      event.preventDefault();
      event.stopPropagation();
      this.setState({expanded: false});
    }
  };

  private onSuggestionMouseDown = (event: React.MouseEvent) => {
    event.preventDefault();
  };

  private onSelect = (removal: number) => {
    this.setState({removal});
  };

  private onRemove = () => {
    this.setState(state => ({
      selected: state.selected.filter((_, i) => i !== state.removal),
      removal: -1, error: ''
    }));
  };

  private onUpload = () => {
    this.upload.current.click();
  };

  private onFileSelected = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    event.target.value = '';
    if(!file) {
      return;
    }
    this.cancelImport();
    const version = this.importVersion;
    const reader = new FileReader();
    this.reader = reader;
    this.setState({importing: true, error: ''});
    reader.onload = async () => {
      this.reader = null;
      try {
        const entries = parseCsv(reader.result as string);
        const imported: T[] = [];
        const model = this.props.model;
        for(const entry of entries) {
          const item = await model.parse(entry);
          if(version !== this.importVersion) {
            return;
          }
          if(item === null) {
            throw new Error(`Unknown or ambiguous item: ${entry}`);
          }
          imported.push(item);
        }
        this.setState(state => ({
          selected: this.normalize([...state.selected, ...imported]),
          importing: false, error: '', removal: -1, highlighted: 0
        }));
      } catch(error) {
        if(version !== this.importVersion) {
          return;
        }
        let message = 'The file could not be imported.';
        if(error instanceof Error) {
          message = error.message;
        }
        this.setState({importing: false, error: `Import failed. ${message}`});
      }
    };
    reader.onerror = () => {
      this.reader = null;
      this.setState({importing: false, error: 'The file could not be read.'});
    };
    reader.readAsText(file);
  };

  private static nextIdentifier = 0;
  private dialog: React.RefObject<HTMLDialogElement>;
  private input: React.RefObject<HTMLInputElement>;
  private upload: React.RefObject<HTMLInputElement>;
  private reader: FileReader;
  private searchVersion: number;
  private importVersion: number;
  private searchTimer: number;
  private identifier: string;
}

export namespace EditListModal {

  /** The number of entries permitted in the selection. */
  export enum SelectionMode {
    SINGLE,
    MULTIPLE
  }
}

interface ContentProperties {
  readOnly: boolean;
  disabled: boolean;
  onClose: () => void;
  onSubmit: () => void;
  children: React.ReactNode;
}

class Content extends React.Component<ContentProperties> {
  public render(): JSX.Element {
    return (
      <div className={css(STYLES.content)}>
        {this.props.children}
        <div className={css(STYLES.submission)}>
          {(() => {
            if(this.props.readOnly) {
              return <Button label='OK' style={{width: '100%'}}
                onClick={this.props.onClose}/>;
            }
            return <Button label='Submit' style={{width: '100%'}}
              disabled={this.props.disabled} onClick={this.props.onSubmit}/>;
          })()}
        </div>
      </div>);
  }
}

function Form(props: {readOnly: boolean; children: React.ReactNode}):
    JSX.Element {
  return <div className={css(!props.readOnly && STYLES.form)}>
    {props.children}
  </div>;
}

function Section(props: {readOnly: boolean; single: boolean; heading: string;
    children: React.ReactNode}): JSX.Element {
  return (
    <section aria-label={props.heading}
        className={css(STYLES.section,
          props.single && STYLES.sectionSingle,
          props.readOnly && STYLES.sectionReadonly)}>
      {!props.readOnly &&
        <header className={css(STYLES.listHeader)}>
          <h2 className={css(STYLES.heading)}>{props.heading}</h2>
        </header>}
      {props.children}
    </section>);
}

interface ItemsListProperties<T> {
  items: readonly T[];
  getLabel: (item: T) => string;
  readOnly: boolean;
  selection: number;
  onSelect: (index: number) => void;
}

class ItemsList<T> extends React.Component<ItemsListProperties<T>> {
  public render(): JSX.Element {
    return (
      <ul className={css(STYLES.items)}>
        {this.props.items.map((item, i) =>
          <li key={i}>
            {(() => {
              if(this.props.readOnly) {
                return <div className={css(STYLES.item)}>
                  {this.props.getLabel(item)}
                </div>;
              }
              return <button type='button'
                  aria-pressed={this.props.selection === i}
                  className={css(STYLES.item, STYLES.itemButton)}
                  onClick={() => this.props.onSelect(i)}>
                {this.props.getLabel(item)}
              </button>;
            })()}
          </li>)}
      </ul>);
  }
}

interface ActionsProperties {
  single: boolean;
  removal: number;
  importing: boolean;
  onRemove: () => void;
  onUpload: () => void;
}

class Actions extends React.Component<ActionsProperties> {
  public render(): JSX.Element {
    return (
      <div className={css(STYLES.actions)}>
        <button type='button' aria-label='Remove'
            className={css(STYLES.action)} disabled={this.props.removal === -1}
            onClick={this.props.onRemove}>
          <img src='resources/components/edit_list_modal/remove.svg'
            width='16' height='16' alt=''/>
          <span className={css(STYLES.actionLabel)}>Remove</span>
        </button>
        {!this.props.single &&
          <button type='button' aria-label='Upload CSV'
              className={css(STYLES.action)} disabled={this.props.importing}
              onClick={this.props.onUpload}>
            <img src='resources/components/edit_list_modal/upload.svg'
              width='16' height='16' alt=''/>
            <span className={css(STYLES.actionLabel)}>Upload</span>
          </button>}
      </div>);
  }
}

const STYLES = StyleSheet.create({
  dialog: {
    boxSizing: 'border-box',
    border: 'none',
    padding: 0,
    color: '#333333',
    background: '#FFFFFF',
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    maxWidth: '100vw',
    maxHeight: '100dvh',
    overflowY: 'auto',
    '::backdrop': {
      background: 'rgba(255, 255, 255, 0.9)'
    },
    '@media (max-width: 767px)': {
      position: 'fixed',
      top: 0,
      right: 0,
      bottom: 0,
      left: 'auto',
      margin: 0,
      width: '282px',
      height: '100dvh',
      boxShadow: '-3px 0 6px rgb(0 0 0 / 40%)'
    },
    '@media (min-width: 768px)': {
      width: '300px',
      margin: 'auto',
      borderRadius: '1px',
      boxShadow: '0 0 6px rgb(0 0 0 / 40%)'
    }
  },
  header: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'space-between',
    gap: '12px',
    padding: '18px 18px 30px',
    fontSize: '1rem'
  },
  close: {
    display: 'flex',
    padding: 0,
    border: 0,
    background: 'transparent',
    cursor: 'pointer',
    ':focus-visible': {outline: '2px solid #684BC7'}
  },
  content: {padding: '0 18px 18px'},
  submission: {paddingTop: '30px'},
  form: {borderBottom: '1px solid #E6E6E6', paddingBottom: '30px'},
  search: {position: 'relative', marginBottom: '18px'},
  suggestions: {
    position: 'absolute',
    zIndex: 2,
    left: 0,
    right: 0,
    top: '100%',
    maxHeight: '170px',
    overflowY: 'auto',
    listStyle: 'none',
    padding: 0,
    margin: '4px 0 0',
    background: '#FFFFFF',
    boxShadow: '0 2px 5px rgb(0 0 0 / 30%)'
  },
  suggestion: {
    padding: '9px 10px',
    cursor: 'pointer',
    ':hover': {background: '#F8F8F8'},
    ':is([aria-selected="true"])': {background: '#684BC7', color: '#FFFFFF'}
  },
  noMatches: {padding: '9px 10px'},
  section: {
    boxSizing: 'border-box',
    height: '246px',
    overflowY: 'auto',
    border: '1px solid #C8C8C8'
  },
  sectionSingle: {height: '76px'},
  sectionReadonly: {height: '342px'},
  listHeader: {
    position: 'sticky',
    top: 0,
    background: '#FFFFFF',
    borderBottom: '1px solid #C8C8C8',
    padding: '12px 10px'
  },
  heading: {margin: 0, color: '#4B23A0', fontSize: '0.875rem', fontWeight: 500},
  items: {listStyle: 'none', padding: 0, margin: 0},
  item: {
    boxSizing: 'border-box',
    width: '100%',
    minHeight: '34px',
    padding: '9px 10px',
    overflowWrap: 'anywhere'
  },
  itemButton: {
    display: 'block',
    border: 0,
    font: 'inherit',
    textAlign: 'left',
    color: 'inherit',
    background: 'transparent',
    cursor: 'pointer',
    ':hover': {background: '#F8F8F8'},
    ':focus-visible': {outline: '2px solid #684BC7', outlineOffset: '-2px'},
    ':is([aria-pressed="true"])': {background: '#684BC7', color: '#FFFFFF'}
  },
  actions: {
    display: 'flex',
    justifyContent: 'space-evenly',
    paddingTop: '30px'
  },
  action: {
    display: 'flex',
    alignItems: 'center',
    border: 0,
    padding: 0,
    font: 'inherit',
    color: 'inherit',
    background: 'transparent',
    cursor: 'pointer',
    ':disabled': {opacity: 0.4, cursor: 'default'},
    ':focus-visible': {outline: '1px solid #684BC7', outlineOffset: '4px'}
  },
  actionLabel: {
    paddingLeft: '8px',
    '@media (max-width: 767px)': {display: 'none'}
  },
  error: {color: '#E63F44', overflowWrap: 'anywhere', marginBottom: 0}
});
