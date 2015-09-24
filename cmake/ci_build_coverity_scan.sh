#!/bin/bash

set -e

# Environment check
echo -e "\033[33;1mNote: COVERITY_SCAN_PROJECT_NAME and COVERITY_SCAN_TOKEN are available on Project Settings page on scan.coverity.com\033[0m"
[ -z "$COVERITY_SCAN_PROJECT_NAME" ] && echo "ERROR: COVERITY_SCAN_PROJECT_NAME must be set" && exit 1
[ -z "$COVERITY_SCAN_NOTIFICATION_EMAIL" ] && echo "ERROR: COVERITY_SCAN_NOTIFICATION_EMAIL must be set" && exit 1
[ -z "$COVERITY_SCAN_BUILD_COMMAND" ] && echo "ERROR: COVERITY_SCAN_BUILD_COMMAND must be set" && exit 1
[ -z "$COVERITY_SCAN_TOKEN" ] && echo "ERROR: COVERITY_SCAN_TOKEN must be set" && exit 1

PLATFORM=linux-64
TOOL_ARCHIVE=/tmp/coverity_tool.tgz
TOOL_URL=https://scan.coverity.com/download/${PLATFORM}
TOOL_BASE=/tmp/coverity-scan-analysis
UPLOAD_URL="https://scan.coverity.com/builds"
SCAN_URL="https://scan.coverity.com"

# Verify upload is permitted
AUTH_RES=`curl -s --form project="$COVERITY_SCAN_PROJECT_NAME" --form token="$COVERITY_SCAN_TOKEN" $SCAN_URL/api/upload_permitted`
if [ "$AUTH_RES" = "Access denied" ]; then
  echo -e "\033[33;1mCoverity Scan API access denied. Check COVERITY_SCAN_PROJECT_NAME and COVERITY_SCAN_TOKEN.\033[0m"
  exit 1
else
  AUTH=`echo $AUTH_RES | ruby -e "require 'rubygems'; require 'json'; puts JSON[STDIN.read]['upload_permitted']"`
  if [ "$AUTH" = "true" ]; then
    echo -e "\033[33;1mCoverity Scan analysis authorized per quota.\033[0m"
  else
    WHEN=`echo $AUTH_RES | ruby -e "require 'rubygems'; require 'json'; puts JSON[STDIN.read]['next_upload_permitted_at']"`
    echo -e "\033[33;1mCoverity Scan analysis NOT authorized until $WHEN.\033[0m"
    exit 0
  fi
fi

if [ ! -d $TOOL_BASE ]; then

  # Download Coverity Scan Analysis Tool
  if [ ! -e $TOOL_ARCHIVE ]; then
    echo -e "\033[33;1mDownloading Coverity Scan Analysis Tool...\033[0m"
	wget https://scan.coverity.com/download/linux-64 --post-data "token=$COVERITY_SCAN_TOKEN&project=$COVERITY_SCAN_PROJECT_NAME" -O $TOOL_ARCHIVE
	md5sum $TOOL_ARCHIVE
  fi

  # Extract Coverity Scan Analysis Tool
  echo -e "\033[33;1mExtracting Coverity Scan Analysis Tool...\033[0m"
  mkdir -p $TOOL_BASE
  pushd $TOOL_BASE
  tar xzf $TOOL_ARCHIVE
  popd
fi

TOOL_DIR=`find $TOOL_BASE -type d -name 'cov-analysis*'`

mkdir -p $(pwd)/../coverity
cp -Rf $TOOL_DIR/* $(pwd)/../coverity

# Build
echo -e "\033[33;1mRunning Coverity Scan Analysis Tool...\033[0m"
#COV_BUILD_OPTIONS=""
COV_BUILD_OPTIONS="--parse-error-threshold 85"
RESULTS_DIR=cov-int
make clean
rm CMakeCache.txt 2> /dev/null
rm -Rf CMakeFiles/ 2> /dev/null
export CC=gcc-4.9
export CXX=g++-4.9
../coverity/bin/cov-configure --gcc
eval "${COVERITY_SCAN_BUILD_COMMAND_PREPEND}"
COVERITY_UNSUPPORTED=0 ../coverity/bin/cov-build --dir $RESULTS_DIR $COV_BUILD_OPTIONS $COVERITY_SCAN_BUILD_COMMAND
../coverity/bin/cov-import-scm --dir $RESULTS_DIR --scm git --log $RESULTS_DIR/scm_log.txt 2>&1

# Upload results
echo -e "\033[33;1mTarring Coverity Scan Analysis results...\033[0m"
RESULTS_ARCHIVE=analysis-results.tgz
tar czf $RESULTS_ARCHIVE $RESULTS_DIR
SHA=`git rev-parse --short HEAD`
tail cov-int/build-log.txt

echo -e "\033[33;1mUploading Coverity Scan Analysis results...\033[0m"
response=$(curl \
  --silent --write-out "\n%{http_code}\n" \
  --form project=$COVERITY_SCAN_PROJECT_NAME \
  --form token=$COVERITY_SCAN_TOKEN \
  --form email=$COVERITY_SCAN_NOTIFICATION_EMAIL \
  --form file=@$RESULTS_ARCHIVE \
  --form version=$SHA \
  --form description="Travis CI build" \
  $UPLOAD_URL)
status_code=$(echo "$response" | sed -n '$p')
if [ "$status_code" != "201" ]; then
  TEXT=$(echo "$response" | sed '$d')
  echo -e "\033[33;1mCoverity Scan upload failed: $TEXT.\033[0m"
  exit 1
fi

