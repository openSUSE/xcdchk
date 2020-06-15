Compare_pkglist()
{
    local found

    OLD_RPM="$DATABASE/$PRODUCT/$OLDBUILD/$ARCH/RPM"
    WORKDIR="$DATABASE/$PRODUCT/$BUILD/$ARCH/tmp.pkglist.$$"

    mkdir -p $WORKDIR

    Verbose "Compare package changes between $OLDBUILD and $BUILD"

    rm -f "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-unchanged-RPMs"
    rm -f "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-updated-RPMs"
    rm -f "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-downgraded-RPMs"
    rm -f "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-new-RPMs"
    rm -f "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-arch-changed-RPMs"
    rm -f "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-missing-RPMs"

    for i in $DATA_RPM/* ; do
        local RPM_NAME=${i##*/}

	if [ -f $OLD_RPM/$RPM_NAME ]; then
	    echo $RPM_NAME >> "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-unchanged-RPMs"
	    touch "$WORKDIR"/"$RPM_NAME"
	else
	    local j
	    local RPM_ARCH=${RPM_NAME##*.}
	    local PKGNAME=${RPM_NAME%-*-*}
	    local LIST=`/bin/ls "$OLD_RPM"/"$PKGNAME"-*-* 2>/dev/null`
	    local found

	    found=0

	    if [ X"$LIST" != X ]; then
		for j in $LIST ; do
		    local T2=${j##*/}
		    local T_ARCH=${T2##*.}
		    local T=${T2%-*-*}
		    if [ "$PKGNAME.$RPM_ARCH" = "$T.$T_ARCH" ]; then
			local VER1=${RPM_NAME#$PKGNAME-}; VER1=${VER1%.*}
			local VER2=${T2#$PKGNAME-}; VER2=${VER2%.*}
                        $RPMVERCMP_CMD $VER2 $VER1
                        local CMPRES=$?
                        if [ $CMPRES -eq 1 ] || [ $CMPRES -eq 11 ] ; then
                          echo "$PKGNAME.$RPM_ARCH: $VER2 => $VER1" >> "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-downgraded-RPMs"
                        else
			  echo "$PKGNAME.$RPM_ARCH: $VER2 => $VER1" >> "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-updated-RPMs"
                        fi
			touch $WORKDIR/$T2
			found=1
			break
		    fi
		done
	    fi


            # Check if the arch has changed:
	    if [ $found -eq 0 ]; then
	      if [ X"$LIST" != X ]; then
		for j in $LIST ; do
		    local T2=${j##*/}
		    local T_ARCH=${T2##*.}
		    local T=${T2%-*-*}
		    if [ "$PKGNAME" = "$T" ]; then
                        # Check in every case, if we have a version downgrad, independent of arch change.
			local VER1=${RPM_NAME#$PKGNAME-}; VER1=${VER1%.*}
			local VER2=${T2#$PKGNAME-}; VER2=${VER2%.*}
                        $RPMVERCMP_CMD $VER2 $VER1
                        local CMPRES=$?
                        if [ $CMPRES -eq 1 ] || [ $CMPRES -eq 11 ] ; then
                          echo "$PKGNAME.$RPM_ARCH: $VER2 => $VER1" >> "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-downgraded-RPMs"
                        else
			  echo "$PKGNAME.$RPM_ARCH: $VER2 => $VER1" >> "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-updated-RPMs"
                        fi
			# log arch change.
                        echo "$PKGNAME: $T_ARCH => $RPM_ARCH" >> "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-arch-changed-RPMs"
                        touch $WORKDIR/$T2
			found=1
			break
		    fi
		done
              fi
	    fi


	    if [ $found -eq 0 ]; then
		echo $RPM_NAME >> "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-new-RPMs"
	    fi
	fi
    done


    for i in $OLD_RPM/* ; do
        local RPM_NAME=${i##*/}
	if [ ! -f $WORKDIR/$RPM_NAME ]; then
	    echo $RPM_NAME >> "$DATABASE/$PRODUCT/$BUILD/$ARCH/$OLDBUILD-$BUILD-missing-RPMs"
	fi
    done

    rm -rf $WORKDIR
}
