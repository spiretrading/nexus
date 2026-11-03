from decimal import Decimal, ROUND_HALF_UP
from xml.sax.saxutils import escape

from reportlab.lib import colors
from reportlab.lib.enums import TA_RIGHT
from reportlab.lib.pagesizes import letter
from reportlab.lib.styles import ParagraphStyle
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.cidfonts import UnicodeCIDFont
from reportlab.platypus import (
  Image, Paragraph, SimpleDocTemplate, Spacer, Table, TableStyle)


def format_fee(value):
  amount = value.quantize(Decimal('0.01'), rounding=ROUND_HALF_UP)
  return format(amount, ',.2f')


def round_fees(fees):
  cent = Decimal('0.01')
  rounded = [fee.quantize(cent, rounding=ROUND_HALF_UP) for fee in fees]
  total = sum(fees, Decimal('0')).quantize(cent, rounding=ROUND_HALF_UP)
  difference = total - sum(rounded, Decimal('0'))
  while difference != 0:
    if difference > 0:
      index = max(range(len(fees)), key=lambda i: fees[i] - rounded[i])
      rounded[index] += cent
      difference -= cent
    else:
      index = max(range(len(fees)), key=lambda i: rounded[i] - fees[i])
      rounded[index] -= cent
      difference += cent
  return rounded


def write_statement(output, key, statement, company):
  account_id, billing_date = key
  pdfmetrics.registerFont(UnicodeCIDFont('STSong-Light'))
  details = statement['details']
  ink = colors.HexColor('#182B40')
  muted = colors.HexColor('#596775')
  body = ParagraphStyle('Body', fontName='Helvetica', fontSize=10,
    leading=15, textColor=ink)
  label = ParagraphStyle('Label', parent=body, fontSize=9, textColor=muted)
  account_label = ParagraphStyle('AccountLabel', parent=label, fontSize=8.5,
    leading=11)
  account_value = ParagraphStyle('AccountValue', parent=body, fontSize=9,
    leading=11)
  heading = ParagraphStyle('Heading', parent=body, fontName='Helvetica-Bold',
    fontSize=11, leading=16, alignment=TA_RIGHT)
  address = ParagraphStyle('Address', parent=label, alignment=TA_RIGHT)
  subheading = ParagraphStyle('Subheading', parent=body, fontSize=12,
    leading=18)
  right = ParagraphStyle('Right', parent=body, alignment=TA_RIGHT)
  bold = ParagraphStyle('Bold', parent=body, fontName='Helvetica-Bold')
  total_style = ParagraphStyle('Total', parent=bold, alignment=TA_RIGHT,
    fontSize=12, leading=18)

  def paragraph(value, style):
    text = str(value)
    if any(ord(character) > 255 for character in text):
      style = ParagraphStyle('Unicode', parent=style, fontName='STSong-Light')
    return Paragraph('<br/>'.join(escape(line) for line in text.split('\n')),
      style)

  document = SimpleDocTemplate(output, pagesize=letter,
    rightMargin=48, leftMargin=48, topMargin=48, bottomMargin=48,
    title='Spire statement - %s - %s' %
      (details['account_name'], billing_date.isoformat()),
    author=company['name'])
  logo = ''
  if company.get('logo') is not None:
    logo = Image(str(company['logo']))
    width, height = logo.imageWidth, logo.imageHeight
    logo.drawWidth = 190
    logo.drawHeight = height * logo.drawWidth / width
    logo.hAlign = 'LEFT'
  supplier = [paragraph(company['name'], heading)]
  supplier.extend(paragraph(line, address)
    for line in company.get('address', []))
  for registration in company.get('registrations', []):
    supplier.extend([paragraph(registration['label'] + ':', address),
      paragraph(registration['value'], address)])
  header = Table([[logo, supplier]], colWidths=[250, 266], hAlign='LEFT')
  header.setStyle(TableStyle([
    ('VALIGN', (0, 0), (-1, -1), 'TOP'),
    ('LEFTPADDING', (0, 0), (-1, -1), 0),
    ('RIGHTPADDING', (0, 0), (-1, -1), 0),
    ('TOPPADDING', (0, 0), (-1, -1), 0),
    ('BOTTOMPADDING', (0, 0), (-1, -1), 0)]))
  story = [header, Spacer(1, 24),
    paragraph('Spire Licence Fee Statement', subheading), Spacer(1, 18)]
  fields = [
    ('Billing date', billing_date.isoformat()),
    ('Trading period', '%s to %s' %
      (details['trading_period_start'], details['trading_period_end'])),
    ('Account name', details['account_name']),
    ('Account holder', details['account_holder_name']),
    ('Email', details['account_email']),
    ('Address', details['address'])]
  information = Table([[paragraph(name, account_label),
    paragraph(value, account_value)]
    for name, value in fields if value or name == 'Address'],
    colWidths=[100, 416], hAlign='LEFT')
  information.setStyle(TableStyle([
    ('VALIGN', (0, 0), (-1, -1), 'TOP'),
    ('LEFTPADDING', (0, 0), (-1, -1), 0),
    ('TOPPADDING', (0, 0), (-1, -1), 1),
    ('BOTTOMPADDING', (0, 0), (-1, -1), 1)]))
  story.extend([information, Spacer(1, 18)])
  amounts = [[paragraph('Description', bold),
    paragraph('Currency', bold), paragraph('Amount', right)]]
  rounded_fees = [Decimal('0')] * len(statement['items'])
  for currency in statement['totals']:
    indices = [i for i, item in enumerate(statement['items'])
      if item[1] == currency]
    fees = [statement['items'][i][2] for i in indices]
    for i, amount in zip(indices, round_fees(fees)):
      rounded_fees[i] = amount
  for (description, currency, fee), amount in zip(
      statement['items'], rounded_fees):
    amounts.append([paragraph(description, body), paragraph(currency, body),
      paragraph(format_fee(amount), right)])
  first_total = len(amounts)
  for currency, total in statement['totals'].items():
    amounts.append([paragraph('Total charges', bold),
      paragraph(currency, bold), paragraph(format_fee(total), total_style)])
  charges = Table(amounts, colWidths=[306, 80, 130], repeatRows=1,
    hAlign='LEFT')
  charges.setStyle(TableStyle([
    ('VALIGN', (0, 0), (-1, -1), 'TOP'),
    ('BACKGROUND', (0, 0), (-1, 0), colors.HexColor('#EDF1F5')),
    ('LINEBELOW', (0, 0), (-1, 0), 0.5, colors.HexColor('#CAD3DD')),
    ('LINEABOVE', (0, first_total), (-1, first_total), 0.7, ink),
    ('LEFTPADDING', (0, 0), (-1, -1), 10),
    ('RIGHTPADDING', (0, 0), (-1, -1), 10),
    ('TOPPADDING', (0, 0), (-1, -1), 10),
    ('BOTTOMPADDING', (0, 0), (-1, -1), 10),
    ('NOSPLIT', (0, first_total - 1), (-1, -1))]))
  story.append(charges)

  def footer(canvas, document):
    canvas.saveState()
    canvas.setFont('Helvetica', 8)
    canvas.setFillColor(muted)
    canvas.drawString(48, 28, 'Billing date %s' % billing_date.isoformat())
    canvas.drawRightString(564, 28, 'Page %s' % document.page)
    canvas.restoreState()

  document.build(story, onFirstPage=footer, onLaterPages=footer)
