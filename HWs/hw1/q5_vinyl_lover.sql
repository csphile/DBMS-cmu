with newtable(name, format_name, art_name, year, month, day) as (
	select distinct release.name,
		medium_format.name,
		artist_credit_name.name,
		release_info.date_year,
		release_info.date_month,
		release_info.date_day
	from release,
		release_info,
		medium,
		medium_format,
		artist_credit,
		artist_credit_name
	where release_info.release = release.id
		and release.id = medium.release
		and medium_format.id = medium.format
		and artist_credit.id = release.artist_credit
		and artist_credit.id = artist_credit_name.artist_credit
)
select distinct name
from newtable
where format_name like "%Vinyl"
	and art_name = "Coldplay"
order by year,
	month,
	day;