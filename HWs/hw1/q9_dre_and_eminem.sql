with duos(a1, a2, count) as(
    select a1.name,
        a2.name,
        count(*)
    from artist_credit_name acn1
        inner join artist_credit_name acn2 on acn1.artist_credit = acn2.artist_credit
        inner join artist a1 on a1.id = acn1.artist
        inner join artist a2 on a2.id = acn2.artist
        inner join artist_credit ac1 on ac1.id = acn1.artist_credit
        inner join release on release.artist_credit = ac1.id
        inner join language on language.id = release.language
    where acn1.name < acn2.name
        and a1.type = 1
        and a2.type = 1
        and language.name = 'English'
        and a1.begin_date_year > 1960
        and a2.begin_date_year > 1960
    group by a1.id,
        a2.id
),
desc_duos(rank, name1, name2, count) as (
    select row_number() over (
            order by count desc,
                a1,
                a2
        ),
        a1,
        a2,
        count
    from duos
)
select *
from desc_duos
where name1 = "Dr. Dre"
    and name2 = "Eminem";