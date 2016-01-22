local_dir=$(dirname $0)
cd $local_dir
date_time=$(date '+%Y-%m-%d')
cat config.yml | sed "s/start: [0-9]*-[0-9]*-[0-9]* [0-9]*:[0-9]*:[0-9]*/start: $date_time 00:00:00/" > config.yml.new
mv config.yml.new config.yml
cat config.yml | sed "s/end: [0-9]*-[0-9]*-[0-9]* [0-9]*:[0-9]*:[0-9]*/end: $date_time 23:59:59/" > config.yml.new
mv config.yml.new config.yml
./ProfitAndLossReport | \
  mail -a "From: Reporting <reports@domain.com>" \
       -a "Subject: EOD Report" \
       -a "X-Custom-Header: yes" recipients@domain.com
